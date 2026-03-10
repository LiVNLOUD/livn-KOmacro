/*
 * App.cpp — Ana uygulama orkestratörü
 *
 * 4 thread paralel çalışır:
 *   1. Vision:  Ekran yakala → ROI kes → Pixel analiz + OCR → State güncelle
 *   2. Macro:   State oku → Karar ver → Combo/skill tetikle → Input gönder
 *   3. Hotkey:  Hold/Toggle shortcut dinle → Macro'ları aç/kapa
 *   4. HUD:     Overlay pencere güncelle
 *
 * Vision ve Macro arasında paylaşılan GameState struct'ı mutex ile korunur.
 */

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include "app.h"
#include "config.h"
#include "calibration.h"
#include "hotkey.h"
#include "logger.h"
#include "../vision/screen_capture.h"
#include "../vision/pixel_analyzer.h"
#include "../vision/ocr_engine.h"
#include "../vision/roi_manager.h"
#include "../vision/icon_tracker.h"
#include "../input/interception_driver.h"
#include "../macro/macro_engine.h"
#include "../ui/hud.h"

#include <Windows.h>
#include <iostream>
#include <chrono>

namespace Livn {

App::App() = default;
App::~App() { shutdown(); }

bool App::init(const std::string& configPath) {
    // 1. Config yükle
    m_config = std::make_unique<Config>();
    if (!m_config->load(configPath)) {
        std::cerr << "[HATA] Config yüklenemedi: " << configPath << std::endl;
        return false;
    }

    m_logger = std::make_unique<Logger>();
    m_logger->info("Livn Macro v4.1 başlatılıyor...");

    // 2. Hedef pencereyi bul
    std::string exeName = m_config->getString("system.target_exe", "KnightOnLine.exe");
    m_logger->info("Hedef EXE: " + exeName);

    // Pencere başlığı ile bul
    m_targetWindow = FindWindowA(nullptr, "Knight OnLine Client");

    if (!m_targetWindow) {
        // Başlık bulunamazsa tüm masaüstünü yakala (test modu)
        m_logger->warn("Oyun penceresi bulunamadi — masaustu modu");
        m_targetWindow = GetDesktopWindow();
    } else {
        m_logger->info("Oyun penceresi bulundu!");
    }

    // 3. Vision modülleri
    m_screenCap = std::make_unique<ScreenCapture>();
    if (!m_screenCap->init(m_targetWindow)) {
        m_logger->error("Ekran yakalama başlatılamadı");
        return false;
    }

    m_calibration = std::make_unique<Calibration>();
    m_calibration->load(m_config.get());

    m_roiManager = std::make_unique<RoiManager>(m_calibration.get());

    // Debug mod: config'de "system.roi_debug": true ise ROI loglanır
    if (m_config->getBool("system.roi_debug", false)) {
        m_roiManager->setDebugMode(true);
        m_logger->info("[Vision] ROI debug modu aktif");
    }

    m_pixelAnalyzer = std::make_unique<PixelAnalyzer>();
    m_iconTracker = std::make_unique<IconTracker>(m_pixelAnalyzer.get(), m_roiManager.get());

    m_ocr = std::make_unique<OcrEngine>();
    if (!m_ocr->init("eng")) { // Tesseract dil dosyası
        m_logger->warn("OCR başlatılamadı — mob ismi okuma devre dışı");
    }

    // 4. Input modülleri
    m_humanizer = std::make_unique<Humanizer>(
        m_config->getInt("system.jitter_ms", 5),
        m_config->getBool("system.humanize", true)
    );

    std::string driverMode = m_config->getString("system.driver_mode", "interception");
    if (driverMode == "interception") {
        m_driver = std::make_unique<InterceptionDriver>();
        if (!m_driver->init()) {
            m_logger->warn("Interception driver yüklenemedi — SendInput'a düşülüyor");
            m_driver.reset();
        }
    }

    m_keySender = std::make_unique<KeySender>(m_driver.get(), m_humanizer.get());
    m_mouseSender = std::make_unique<MouseSender>(m_driver.get(), m_humanizer.get());

    // 5. Macro engine
    m_macroEngine = std::make_unique<MacroEngine>(
        m_config.get(), m_keySender.get(), m_mouseSender.get(),
        m_iconTracker.get(), m_pixelAnalyzer.get(), m_ocr.get(),
        m_roiManager.get(), m_logger.get()
    );

    // 5b. İkon takibini config'den otomatik kur
    // skill_bar kalibre edildikten sonra setupIcons() tekrar çağrılabilir.
    // Şimdilik başlangıçta deniyoruz — kalibrasyon yoksa boş rect gelir, atlanır.
    {
        auto setupIcon = [&](const std::string& id, const std::string& path) {
            auto slot = m_config->getSkillSlot(path);
            auto barROI = m_calibration->getROI("skill_bar");
            if (!barROI.calibrated) return;
            cv::Rect barRect(barROI.x, barROI.y, barROI.w, barROI.h);
            m_iconTracker->autoSetupIcon(id, slot.fTab, slot.slot, barRect);
            m_logger->info("[IconTracker] " + id + " → " + slot.fTab + "/" + slot.slot);
        };

        // Priest
        setupIcon("kitap", "priest.bp_attack.kitap");
        setupIcon("kol",   "priest.bp_attack.kol");
        // Warrior buffs
        setupIcon("kafa",  "warrior.buff_tracking.kafa");
        setupIcon("kilic", "warrior.buff_tracking.kilic");
        setupIcon("kol_w", "warrior.buff_tracking.kol");
        setupIcon("hpb",   "warrior.buff_tracking.hp_booster");
        // Rogue
        setupIcon("lf",    "rogue.vs_combo.lf_skill");
        setupIcon("def",   "rogue.auto_def.skill");
    }

    // 6. Hotkey manager
    m_hotkeys = std::make_unique<HotkeyManager>(m_config.get(), m_macroEngine.get());

    // 7. HUD
    if (m_config->getBool("extra.hud.enabled", true)) {
        m_hud = std::make_unique<HudOverlay>();
    }

    m_logger->info("Başlatma tamamlandı. Thread'ler başlıyor...");
    return true;
}

void App::run() {
    m_running = true;

    m_visionThread = std::thread(&App::visionLoop, this);
    m_macroThread  = std::thread(&App::macroLoop, this);
    m_hotkeyThread = std::thread(&App::hotkeyLoop, this);

    if (m_hud) {
        m_hudThread = std::thread(&App::hudLoop, this);
    }

    m_logger->info("Tum thread'ler aktif. Calisiyor...");
    m_logger->info("Durdurmak icin CTRL+C basin.");
    m_logger->info("========================================");

    // Ana thread: konsol bekleme döngüsü
    while (m_running) {
        // ESC tuşuna basılırsa çık
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            m_logger->info("ESC basildi — kapatiliyor...");
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    shutdown();
}

void App::shutdown() {
    m_running = false;

    if (m_visionThread.joinable())  m_visionThread.join();
    if (m_macroThread.joinable())   m_macroThread.join();
    if (m_hotkeyThread.joinable())  m_hotkeyThread.join();
    if (m_hudThread.joinable())     m_hudThread.join();

    m_logger->info("Livn Macro kapatıldı.");
}

// ── Vision Thread: ~60 FPS ekran yakalama + analiz ──
void App::visionLoop() {
    using clock = std::chrono::steady_clock;
    constexpr auto frameTime = std::chrono::milliseconds(16); // ~60fps

    while (m_running) {
        auto start = clock::now();

        // 1. Ekranı yakala
        cv::Mat frame = m_screenCap->capture();
        if (frame.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        // 2. ROI bölgelerini kes ve analiz et
        m_roiManager->updateFrame(frame);

        // 3. HP/MP bar pixel analiz
        m_pixelAnalyzer->analyzeHpBar(m_roiManager->getROI("hp_bar"));
        m_pixelAnalyzer->analyzeMpBar(m_roiManager->getROI("mp_bar"));

        // 4. Parti HP barları
        m_pixelAnalyzer->analyzePartyBars(m_roiManager->getROI("party_bar"));

        // 5. Buff/Debuff ikon takibi
        m_iconTracker->update(m_roiManager->getROI("buff_bar"));
        m_iconTracker->updateSkillBar(m_roiManager->getROI("skill_bar"));

        // 6. Silah kırılma ikonu
        m_pixelAnalyzer->checkWeaponBreak(m_roiManager->getROI("weapon_break"));

        // 7. OCR (her 5 frame'de bir — performans)
        static int ocrCounter = 0;
        if (++ocrCounter >= 5 && m_ocr && m_ocr->isReady()) {
            ocrCounter = 0;
            // Chat OCR (TP algılama)
            m_ocr->processChat(m_roiManager->getROI("chat_area"));
            // Mob ismi OCR (görsel farm)
            // m_ocr->processMobName(...) — aktifse
        }

        // Frame rate limitle
        auto elapsed = clock::now() - start;
        if (elapsed < frameTime) {
            std::this_thread::sleep_for(frameTime - elapsed);
        }
    }
}

// ── Macro Thread: Karar + uygulama döngüsü ──
void App::macroLoop() {
    using clock = std::chrono::steady_clock;
    constexpr auto tickTime = std::chrono::milliseconds(10); // 10ms tick

    while (m_running) {
        auto start = clock::now();

        // Sadece oyun penceresi aktifken çalış
        HWND fg = GetForegroundWindow();
        if (fg == m_targetWindow) {
            m_macroEngine->tick();
        }

        auto elapsed = clock::now() - start;
        if (elapsed < tickTime) {
            std::this_thread::sleep_for(tickTime - elapsed);
        }
    }
}

// ── Hotkey Thread: Hold/Toggle shortcut dinleme ──
void App::hotkeyLoop() {
    while (m_running) {
        m_hotkeys->poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

// ── HUD Thread: Overlay güncelleme ──
void App::hudLoop() {
    while (m_running) {
        if (m_hud) {
            m_hud->update(
                m_pixelAnalyzer->getHpPercent(),
                m_pixelAnalyzer->getMpPercent(),
                m_macroEngine->getActiveComboName(),
                m_iconTracker->getStatus()
            );
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

} // namespace Livn

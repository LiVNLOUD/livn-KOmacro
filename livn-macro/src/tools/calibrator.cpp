/*
 * LivnMacro Calibrator — Görsel ROI kalibrasyon aracı
 *
 * Kullanım:
 *   1. Oyunu aç, tam ekran yap
 *   2. calibrator.exe çalıştır
 *   3. Ekranda şeffaf overlay açılır
 *   4. Her ROI için: fareyle sürükle → dikdörtgen çiz → ENTER ile kaydet
 *   5. Tüm ROI'lar bitti → config/default.json güncellenir
 *   6. ESC ile herhangi bir adımda çık (kaydedilmişler korunur)
 *
 * Kontroller:
 *   Sol tık + sürükle  → ROI dikdörtgeni çiz
 *   ENTER              → Bu ROI'yi kaydet, sıradakine geç
 *   R                  → Bu adımı sıfırla (yeniden çiz)
 *   ESC                → Çık (şimdiye kadar kaydedilenler korunur)
 */

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <nlohmann/json.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

using json = nlohmann::json;

// ── Kalibre edilecek ROI'ların sırası ve açıklamaları ──
struct ROITask {
    std::string key;         // JSON key ("hp_bar" vb.)
    std::string label;       // Ekranda gösterilecek Türkçe açıklama
    std::string hint;        // Alt satır ipucu
    cv::Scalar color;        // Overlay dikdörtgen rengi
};

static const std::vector<ROITask> TASKS = {
    { "hp_bar",       "HP BAR",
      "Kendi HP barini ciz (kirmizi bar)",
      cv::Scalar(0, 60, 220) },

    { "mp_bar",       "MP BAR",
      "Kendi MP barini ciz (mavi bar)",
      cv::Scalar(200, 60, 0) },

    { "party_bar",    "PARTI BARLARI",
      "Tum parti HP barlarini kapsayan alani ciz (8 uyenin tamami)",
      cv::Scalar(0, 180, 60) },

    { "buff_bar",     "BUFF BAR",
      "Buff/debuff ikonlarinin bulundugu satiri ciz",
      cv::Scalar(0, 200, 200) },

    { "skill_bar",    "SKILL BAR",
      "Skill slot barini ciz (aktif F tab'in slotlari)",
      cv::Scalar(220, 120, 0) },

    { "chat_area",    "CHAT ALANI",
      "Sohbet kutusunu ciz (TP komutlari buradan okunacak) — atlayabilirsin (ESC)",
      cv::Scalar(180, 0, 180) },

    { "weapon_break", "SILAH KIRILMA IKONU",
      "Silah kırılma uyarı ikonunun bulundugu koseyi ciz — atlayabilirsin (ESC)",
      cv::Scalar(0, 180, 220) },
};

// ── Global fare state ──
struct MouseState {
    bool drawing = false;
    bool done    = false;
    cv::Point start, end;
};

static MouseState g_mouse;
static cv::Mat    g_display;
static cv::Mat    g_screenshot;

// ── Fare callback ──
static void onMouse(int event, int x, int y, int /*flags*/, void* /*userdata*/) {
    if (event == cv::EVENT_LBUTTONDOWN) {
        g_mouse.drawing = true;
        g_mouse.done    = false;
        g_mouse.start   = { x, y };
        g_mouse.end     = { x, y };
    } else if (event == cv::EVENT_MOUSEMOVE && g_mouse.drawing) {
        g_mouse.end = { x, y };
    } else if (event == cv::EVENT_LBUTTONUP && g_mouse.drawing) {
        g_mouse.drawing = false;
        g_mouse.done    = true;
        g_mouse.end     = { x, y };
    }
}

// ── Ekran görüntüsü al (BitBlt, tüm masaüstü) ──
static cv::Mat captureDesktop() {
    HWND desktop = GetDesktopWindow();
    RECT r;
    GetClientRect(desktop, &r);
    int w = r.right, h = r.bottom;

    HDC hdcScreen = GetDC(desktop);
    HDC hdcMem    = CreateCompatibleDC(hdcScreen);
    HBITMAP hBmp  = CreateCompatibleBitmap(hdcScreen, w, h);
    SelectObject(hdcMem, hBmp);
    BitBlt(hdcMem, 0, 0, w, h, hdcScreen, 0, 0, SRCCOPY);

    BITMAPINFOHEADER bi = {};
    bi.biSize     = sizeof(bi);
    bi.biWidth    = w;
    bi.biHeight   = -h;
    bi.biPlanes   = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;

    cv::Mat frame(h, w, CV_8UC4);
    GetDIBits(hdcMem, hBmp, 0, h, frame.data,
              reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS);

    DeleteObject(hBmp);
    DeleteDC(hdcMem);
    ReleaseDC(desktop, hdcScreen);

    cv::Mat bgr;
    cv::cvtColor(frame, bgr, cv::COLOR_BGRA2BGR);
    return bgr;
}

// ── Overlay çiz ──
static void drawOverlay(const ROITask& task, int stepIdx, int totalSteps,
                        const cv::Rect& currentRect) {
    g_display = g_screenshot.clone();

    // Yarı şeffaf koyu üst panel
    cv::Mat panel = g_display(cv::Rect(0, 0, g_display.cols, 90));
    panel *= 0.35;

    // Adım başlığı
    std::string title = "[" + std::to_string(stepIdx + 1) + "/" +
                        std::to_string(totalSteps) + "]  " + task.label;
    cv::putText(g_display, title,
                { 16, 38 }, cv::FONT_HERSHEY_SIMPLEX, 1.1,
                task.color, 2);

    // İpucu
    cv::putText(g_display, task.hint,
                { 16, 68 }, cv::FONT_HERSHEY_SIMPLEX, 0.55,
                cv::Scalar(220, 220, 220), 1);

    // Kontrol ipucu (sağ üst)
    std::string ctrl = "Sol tik+surukle ciz  |  ENTER kaydet  |  R sifirla  |  ESC cik";
    cv::putText(g_display, ctrl,
                { g_display.cols - 620, 28 },
                cv::FONT_HERSHEY_SIMPLEX, 0.48,
                cv::Scalar(180, 180, 180), 1);

    // Çizilen dikdörtgen
    if (currentRect.width > 2 && currentRect.height > 2) {
        // Dış parlak çerçeve
        cv::rectangle(g_display, currentRect, task.color, 2);
        // İç şeffaf dolgu
        cv::Mat overlay = g_display.clone();
        cv::rectangle(overlay, currentRect, task.color, -1);
        cv::addWeighted(overlay, 0.15, g_display, 0.85, 0, g_display);
        cv::rectangle(g_display, currentRect, task.color, 2);

        // Boyut etiketi
        std::string dim = std::to_string(currentRect.width) + " x " +
                          std::to_string(currentRect.height);
        cv::putText(g_display, dim,
                    { currentRect.x, currentRect.y - 6 },
                    cv::FONT_HERSHEY_SIMPLEX, 0.5,
                    task.color, 1);
    }
}

// ── JSON güncelle ──
static bool saveROI(const std::string& configPath,
                    const std::string& key, const cv::Rect& rect) {
    json data;
    {
        std::ifstream f(configPath);
        if (f.is_open()) f >> data;
    }
    data["calibration"][key] = {
        { "x", rect.x }, { "y", rect.y },
        { "w", rect.width }, { "h", rect.height },
        { "calibrated", true }
    };
    std::ofstream f(configPath);
    if (!f.is_open()) return false;
    f << data.dump(2);
    return true;
}

// ── Normalize rect (sürükleme yönünden bağımsız) ──
static cv::Rect normalizeRect(cv::Point a, cv::Point b) {
    return cv::Rect(
        std::min(a.x, b.x), std::min(a.y, b.y),
        std::abs(a.x - b.x), std::abs(a.y - b.y)
    );
}

int main(int argc, char* argv[]) {
    // Config yolu — exe yanındaki config.json, yoksa üst klasördeki default.json
    std::string configPath = "config.json";
    if (!std::filesystem::exists(configPath))
        configPath = "../../config/default.json";
    if (argc > 1) configPath = argv[1];

    if (!std::filesystem::exists(configPath)) {
        std::cerr << "[HATA] Config bulunamadi: " << configPath << "\n";
        std::cerr << "Kullanim: calibrator.exe [config_yolu]\n";
        return 1;
    }

    std::cout << "== LivnMacro Kalibrator ==\n";
    std::cout << "Config: " << configPath << "\n";
    std::cout << "Ekran goruntüsü alınıyor...\n";

    g_screenshot = captureDesktop();
    if (g_screenshot.empty()) {
        std::cerr << "[HATA] Ekran goruntüsü alinamadi.\n";
        return 1;
    }

    std::cout << "Ekran: " << g_screenshot.cols << "x" << g_screenshot.rows << "\n";
    std::cout << "Pencere açılıyor — lütfen bekleyin...\n\n";

    const std::string WIN = "LivnMacro Kalibrator";
    cv::namedWindow(WIN, cv::WINDOW_NORMAL);
    cv::setWindowProperty(WIN, cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);
    cv::setMouseCallback(WIN, onMouse);

    int totalSteps = static_cast<int>(TASKS.size());
    int step = 0;
    cv::Rect currentRect;

    while (step < totalSteps) {
        const ROITask& task = TASKS[step];

        // Fare state sıfırla
        g_mouse = {};
        currentRect = {};

        std::cout << "--- Adim " << (step + 1) << "/" << totalSteps
                  << ": " << task.label << "\n"
                  << "    " << task.hint << "\n";

        while (true) {
            // Anlık dikdörtgeni hesapla
            if (g_mouse.drawing || g_mouse.done) {
                currentRect = normalizeRect(g_mouse.start, g_mouse.end);
            }

            drawOverlay(task, step, totalSteps, currentRect);
            cv::imshow(WIN, g_display);

            int key = cv::waitKey(16); // ~60fps

            if (key == 27) { // ESC — çık
                std::cout << "\nESC: Kalibrator kapatıldı. "
                          << "Kaydedilenler korunuyor.\n";
                cv::destroyAllWindows();
                return 0;
            }
            if (key == 'r' || key == 'R') { // R — sıfırla
                g_mouse = {};
                currentRect = {};
                std::cout << "  [Sifirlandı]\n";
            }
            if (key == 13 || key == '\r') { // ENTER — kaydet
                if (currentRect.width < 5 || currentRect.height < 5) {
                    std::cout << "  [Uyarı] Cok kucuk dikdortgen — once cizin\n";
                    continue;
                }
                if (saveROI(configPath, task.key, currentRect)) {
                    std::cout << "  [OK] " << task.key << " kaydedildi: "
                              << currentRect.x << "," << currentRect.y
                              << "  " << currentRect.width << "x"
                              << currentRect.height << "\n";
                    step++;
                    break;
                } else {
                    std::cerr << "  [HATA] Config yazılamadı: " << configPath << "\n";
                }
            }
        }
    }

    std::cout << "\n== Kalibrasyon tamamlandı! ==\n";
    std::cout << "Config guncellendi: " << configPath << "\n";
    std::cout << "Simdi LivnMacro.exe calistirabilirsiniz.\n";

    cv::destroyAllWindows();
    return 0;
}

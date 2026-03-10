#pragma once
#include <string>
#include <vector>
#include <atomic>
#include <mutex>
#include "../core/config.h"

namespace Livn {

class KeySender;
class MouseSender;
class IconTracker;
class PixelAnalyzer;
class OcrEngine;
class RoiManager;
class Logger;

/*
 * MacroEngine — Ana karar motoru
 *
 * Her tick() çağrısında:
 * 1. Aktif macro'ları kontrol et (hotkey'den açılmış olanlar)
 * 2. Vision verilerini oku (HP/MP, parti, buff, CD)
 * 3. Karar ver (hangi skill basılmalı?)
 * 4. Input gönder (KeySender/MouseSender ile)
 *
 * Sınıf modülleri: Mage, Priest, Rogue, Warrior, Extra
 * Her biri kendi tick() metodu ile bağımsız çalışır.
 */

enum class ComboState {
    IDLE,       // Çalışmıyor
    RUNNING,    // Hold basılı veya toggle açık
    PAUSED      // Geçici duraklama (CD bekleme vb.)
};

struct ActiveCombo {
    std::string name;
    ComboConfig config;
    ComboState state = ComboState::IDLE;
    int currentStep = 0;
    std::chrono::steady_clock::time_point lastAction;
    bool holdActive = false;    // Hold modu: tuş basılı mı?
    bool toggleActive = false;  // Toggle modu: açık mı?
};

class MacroEngine {
public:
    MacroEngine(Config* config, KeySender* keys, MouseSender* mouse,
                IconTracker* icons, PixelAnalyzer* pixels, OcrEngine* ocr,
                RoiManager* roi, Logger* logger);

    void tick();

    // Hotkey'den çağrılır
    void activateCombo(const std::string& name, bool pressed); // Hold: pressed=true/false
    void toggleCombo(const std::string& name);                  // Toggle: her basışta aç/kapa

    std::string getActiveComboName() const;

private:
    // ── Sınıf modülleri ──
    void tickMageStaff();
    void tickMageNova();
    void tickMageFarm();
    void tickMageTP();

    void tickPriestAttack();
    void tickPriestKitapKol();    // Ataktan bağımsız paralel
    void tickPriestHeal();
    void tickPriestCure();

    void tickRogueVS();
    void tickRogueSlide35();
    void tickRogueSkillCombo();
    void tickRogueDef();

    void tickWarriorDes();
    void tickWarriorSerial();
    void tickWarriorBuffs();
    void tickWarriorFirfir();
    void tickWarriorDebuff();

    void tickExtraHPMP();
    void tickExtraFarm();
    void tickExtraHammer();
    void tickExtraDuration();

    // ── Yardımcı ──
    // Skill basma: F tab'a geç + slot'a bas + delay bekle
    void pressSkill(const SkillSlot& skill);

    // Combo step: sıradaki skill'i bas, delay bekle, sonrakine geç
    bool comboStep(ActiveCombo& combo);

    // Envanter işlemi: I ile envanter aç (kapalıysa) → pozisyona tıkla
    void inventoryAction(int posX, int posY);

    Config* m_config;
    KeySender* m_keys;
    MouseSender* m_mouse;
    IconTracker* m_icons;
    PixelAnalyzer* m_pixels;
    OcrEngine* m_ocr;
    RoiManager* m_roi;
    Logger* m_logger;

    std::unordered_map<std::string, ActiveCombo> m_combos;
    mutable std::mutex m_mutex;
};

} // namespace Livn

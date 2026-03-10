#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include "macro_engine.h"
#include "../input/interception_driver.h"
#include "../vision/icon_tracker.h"
#include "../vision/pixel_analyzer.h"
#include "../vision/ocr_engine.h"
#include "../vision/roi_manager.h"
#include "../core/logger.h"

#include <chrono>

namespace Livn {

using clock = std::chrono::steady_clock;

MacroEngine::MacroEngine(Config* config, KeySender* keys, MouseSender* mouse,
    IconTracker* icons, PixelAnalyzer* pixels, OcrEngine* ocr,
    RoiManager* roi, Logger* logger)
    : m_config(config), m_keys(keys), m_mouse(mouse),
      m_icons(icons), m_pixels(pixels), m_ocr(ocr),
      m_roi(roi), m_logger(logger)
{
    // Config'den combo'ları yükle
    auto loadCombo = [&](const std::string& name, const std::string& path) {
        ActiveCombo ac;
        ac.name = name;
        ac.config = m_config->getCombo(path);
        m_combos[name] = ac;
    };

    loadCombo("mage_staff",     "mage.staff_combo");
    loadCombo("mage_nova",      "mage.nova");
    loadCombo("priest_attack",  "priest.bp_attack");
    loadCombo("rogue_vs",       "rogue.vs_combo");
    loadCombo("rogue_slide35",  "rogue.slide_35");
    loadCombo("rogue_combo",    "rogue.skill_combo");
    loadCombo("warrior_des",    "warrior.des_combo");
    loadCombo("warrior_serial", "warrior.serial_attack");
    loadCombo("warrior_shield", "warrior.shield_spam");
    loadCombo("warrior_firfir", "warrior.firfir");
    loadCombo("extra_farm",     "extra.auto_farm");
}

void MacroEngine::activateCombo(const std::string& name, bool pressed) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_combos.find(name);
    if (it == m_combos.end()) return;

    it->second.holdActive = pressed;
    if (pressed) {
        it->second.state = ComboState::RUNNING;
        it->second.currentStep = 0;
        it->second.lastAction = clock::now();
        m_logger->info("[" + name + "] Hold aktif");
    } else {
        it->second.state = ComboState::IDLE;
        m_logger->info("[" + name + "] Hold bırakıldı");
    }
}

void MacroEngine::toggleCombo(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_combos.find(name);
    if (it == m_combos.end()) return;

    it->second.toggleActive = !it->second.toggleActive;
    it->second.state = it->second.toggleActive ? ComboState::RUNNING : ComboState::IDLE;
    it->second.currentStep = 0;
    it->second.lastAction = clock::now();
    m_logger->info("[" + name + "] Toggle: " + (it->second.toggleActive ? "ON" : "OFF"));
}

bool MacroEngine::comboStep(ActiveCombo& combo) {
    auto now = clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - combo.lastAction).count();

    if (elapsed < combo.config.delayMs) return false; // Delay bekleniyor

    if (combo.config.skills.empty()) return false;

    // Sıradaki skill'i bas
    int idx = combo.currentStep % combo.config.skills.size();
    pressSkill(combo.config.skills[idx]);

    combo.currentStep++;
    combo.lastAction = now;
    return true;
}

void MacroEngine::pressSkill(const SkillSlot& skill) {
    m_keys->pressSkillSlot(skill.getFTabVK(), skill.getSlotVK());
}

void MacroEngine::inventoryAction(int posX, int posY) {
    // TODO: Envanter açık mı pixel kontrolü
    // Kapalıysa I bas
    m_keys->pressKey('I');
    m_keys->pressKey('I'); // Kısa bekleme sonrası
    // Pozisyona tıkla
    m_mouse->clickAt(posX, posY);
}

// ══════════════════════════════════════
//  ANA TICK — Her 1ms çağrılır
// ══════════════════════════════════════
void MacroEngine::tick() {
    std::lock_guard<std::mutex> lock(m_mutex);

    // ── Mage ──
    if (m_combos["mage_staff"].state == ComboState::RUNNING)
        tickMageStaff();
    if (m_combos["mage_nova"].state == ComboState::RUNNING)
        tickMageNova();
    tickMageFarm();
    tickMageTP();

    // ── Priest (Kitap/Kol her zaman çalışır — ataktan bağımsız) ──
    if (m_combos["priest_attack"].state == ComboState::RUNNING)
        tickPriestAttack();
    tickPriestKitapKol();  // Her zaman aktif
    tickPriestHeal();
    tickPriestCure();

    // ── Rogue ──
    if (m_combos["rogue_vs"].state == ComboState::RUNNING)
        tickRogueVS();
    if (m_combos["rogue_slide35"].state == ComboState::RUNNING)
        tickRogueSlide35();
    if (m_combos["rogue_combo"].state == ComboState::RUNNING)
        tickRogueSkillCombo();
    tickRogueDef();

    // ── Warrior ──
    if (m_combos["warrior_des"].state == ComboState::RUNNING)
        tickWarriorDes();
    if (m_combos["warrior_serial"].state == ComboState::RUNNING)
        tickWarriorSerial();
    tickWarriorBuffs();
    if (m_combos["warrior_firfir"].state == ComboState::RUNNING)
        tickWarriorFirfir();
    tickWarriorDebuff();

    // ── Extra ──
    tickExtraHPMP();
    if (m_combos["extra_farm"].state == ComboState::RUNNING)
        tickExtraFarm();
    tickExtraHammer();
    tickExtraDuration();
}

// ══════════════════════════════════════
//  MAGE
// ══════════════════════════════════════
void MacroEngine::tickMageStaff() {
    auto& combo = m_combos["mage_staff"];
    auto now = clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - combo.lastAction).count();

    int delay = m_config->getInt("mage.staff_combo.delay_ms", 300);

    if (elapsed >= delay) {
        m_keys->pressZ();
        combo.currentStep++;
        combo.lastAction = now;

        // Her 10 hit'te bir logla (spam'i önle)
        if (combo.currentStep % 10 == 1) {
            m_logger->info("[MAGE] Staff combo aktif — hit #" + std::to_string(combo.currentStep));
        }
    }
}

void MacroEngine::tickMageNova() {
    comboStep(m_combos["mage_nova"]);
}

void MacroEngine::tickMageFarm() {
    // Görsel farm — toggle ile aktif edildiğinde
    if (!m_config->getBool("mage.visual_farm.enabled")) return;
    // TODO: OCR mob ismi → eşleşirse spam skill
}

void MacroEngine::tickMageTP() {
    if (!m_config->getBool("mage.smart_tp.enabled")) return;

    float threshold = m_config->getInt("mage.smart_tp.hp_threshold", 30);
    auto& party = m_pixels->getPartyState();

    for (int i = 0; i < party.size(); i++) {
        if (party[i].hpPercent < threshold && party[i].hpPercent > 0) {
            // Bu üyeye TP at
            auto tpSkill = m_config->getSkillSlot("mage.smart_tp.skill");
            // Parti listesinden üyeyi seç → TP
            pressSkill(tpSkill);
            m_logger->info("[TP] Üye " + std::to_string(i+1) + " HP %" + std::to_string((int)party[i].hpPercent));
            break;
        }
    }
}

// ══════════════════════════════════════
//  PRIEST
// ══════════════════════════════════════
void MacroEngine::tickPriestAttack() {
    // Skill(2) → R → R döngüsü
    auto& combo = m_combos["priest_attack"];
    auto now = clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - combo.lastAction).count();

    int delay = combo.config.delayMs; // ~300ms USKO optimal

    if (elapsed >= delay) {
        int step = combo.currentStep % 3;
        if (step == 0) {
            auto atkSkill = m_config->getSkillSlot("priest.bp_attack.attack_skill");
            pressSkill(atkSkill);
        } else {
            m_keys->pressR();
        }
        combo.currentStep++;
        combo.lastAction = now;
    }
}

void MacroEngine::tickPriestKitapKol() {
    // Kitap ve Kol: ataktan BAĞIMSIZ, paralel CD takibi
    if (!m_config->getBool("priest.bp_attack.enabled")) return;

    // Kitap CD kontrolü
    if (m_icons->needsRefresh("kitap")) {
        auto kitap = m_config->getSkillSlot("priest.bp_attack.kitap");
        pressSkill(kitap);
        m_icons->resetRefresh("kitap");
        m_logger->info("[PRIEST] Kitap CD bitti → yenilendi");
    }

    // Kol CD kontrolü
    if (m_icons->needsRefresh("kol")) {
        auto kol = m_config->getSkillSlot("priest.bp_attack.kol");
        pressSkill(kol);
        m_icons->resetRefresh("kol");
        m_logger->info("[PRIEST] Kol CD bitti → yenilendi");
    }
}

void MacroEngine::tickPriestHeal() {
    if (!m_config->getBool("priest.heal.enabled")) return;

    float singleTh = m_config->getInt("priest.heal.single_threshold", 60);
    float groupTh = m_config->getInt("priest.heal.group_threshold", 40);
    auto& party = m_pixels->getPartyState();

    int lowCount = 0;
    int lowestIdx = -1;
    float lowestHp = 100;

    for (int i = 0; i < party.size(); i++) {
        if (party[i].hpPercent < singleTh && party[i].hpPercent > 0) {
            lowCount++;
            if (party[i].hpPercent < lowestHp) {
                lowestHp = party[i].hpPercent;
                lowestIdx = i;
            }
        }
    }

    if (lowCount >= 3 || (lowCount >= 2 && lowestHp < groupTh)) {
        // Toplu heal
        auto gheal = m_config->getSkillSlot("priest.heal.group");
        pressSkill(gheal);
    } else if (lowestIdx >= 0) {
        // Tekli heal — en düşük HP'li üyeyi seç → heal
        auto heal = m_config->getSkillSlot("priest.heal.single");
        pressSkill(heal);
    }
}

void MacroEngine::tickPriestCure() {
    if (!m_config->getBool("priest.cure.enabled")) return;

    auto& party = m_pixels->getPartyState();
    for (auto& pm : party) {
        if (pm.hasDebuff) {
            auto cure = m_config->getSkillSlot("priest.cure.skill");
            pressSkill(cure);

            if (pm.debuffColor == 2) {
                // Torment → ek olarak kalkan tak
                auto shieldPos = m_config->getPosition("shield");
                if (shieldPos.recorded) {
                    inventoryAction(shieldPos.x, shieldPos.y);
                }
            }
            break;
        }
    }
}

// ══════════════════════════════════════
//  ROGUE
// ══════════════════════════════════════
void MacroEngine::tickRogueVS() {
    // Minor → R → Skill → R döngüsü
    auto& combo = m_combos["rogue_vs"];
    auto now = clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - combo.lastAction).count();

    if (elapsed < combo.config.delayMs) return;

    int step = combo.currentStep % 4; // minor, R, skill, R
    if (step == 0) {
        auto minor = m_config->getSkillSlot("rogue.vs_combo.minor");
        pressSkill(minor);
    } else if (step == 1 || step == 3) {
        m_keys->pressR();
    } else if (step == 2) {
        if (!combo.config.skills.empty()) {
            int skillIdx = (combo.currentStep / 4) % combo.config.skills.size();
            pressSkill(combo.config.skills[skillIdx]);
        }
    }

    combo.currentStep++;
    combo.lastAction = now;

    // Oto LF kontrolü
    if (m_config->getBool("rogue.vs_combo.auto_lf")) {
        if (m_icons->getIconState("lf") == IconState::GONE) {
            auto lf = m_config->getSkillSlot("rogue.vs_combo.lf_skill");
            pressSkill(lf);
            m_logger->info("[ROGUE] LF yenilendi");
        }
    }
}

void MacroEngine::tickRogueSlide35() {
    // 3'lü ok → W → 5'li ok → W döngüsü
    auto& combo = m_combos["rogue_slide35"];
    auto now = clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - combo.lastAction).count();

    if (elapsed < combo.config.delayMs) return;

    int step = combo.currentStep % 4;
    if (step == 0) {
        auto ok3 = m_config->getSkillSlot("rogue.slide_35.ok3");
        pressSkill(ok3);
    } else if (step == 1 || step == 3) {
        m_keys->pressW(80); // 80ms W basılı tut
    } else if (step == 2) {
        auto ok5 = m_config->getSkillSlot("rogue.slide_35.ok5");
        pressSkill(ok5);
    }

    combo.currentStep++;
    combo.lastAction = now;
}

void MacroEngine::tickRogueSkillCombo() {
    comboStep(m_combos["rogue_combo"]);
}

void MacroEngine::tickRogueDef() {
    if (!m_config->getBool("rogue.auto_def.enabled")) return;
    if (m_icons->needsRefresh("def")) {
        auto def = m_config->getSkillSlot("rogue.auto_def.skill");
        pressSkill(def);
        m_icons->resetRefresh("def");
    }
}

// ══════════════════════════════════════
//  WARRIOR
// ══════════════════════════════════════
void MacroEngine::tickWarriorDes() {
    // Des → F tab değiştir → Yere vur skill'leri sırasıyla → Kalkan
    auto& combo = m_combos["warrior_des"];

    if (combo.currentStep == 0) {
        auto des = m_config->getSkillSlot("warrior.des_combo.descent");
        pressSkill(des);
        combo.currentStep = 1;
        combo.lastAction = clock::now();
        return;
    }

    auto now = clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - combo.lastAction).count();
    if (elapsed < combo.config.delayMs) return; // ~300ms bekleme

    // Yere vur skill'leri
    int groundIdx = combo.currentStep - 1;
    auto groundSkills = m_config->getCombo("warrior.des_combo").skills; // ground_skills

    if (groundIdx < groundSkills.size()) {
        pressSkill(groundSkills[groundIdx]);
        combo.currentStep++;
        combo.lastAction = now;
    } else {
        // Bitirdik — kalkan tak
        auto shieldPos = m_config->getPosition("shield");
        if (shieldPos.recorded) {
            inventoryAction(shieldPos.x, shieldPos.y);
        }
        // Combo bitir (toggle ise kapat)
        combo.state = ComboState::IDLE;
        combo.toggleActive = false;
        combo.currentStep = 0;
        m_logger->info("[WARRIOR] Des combo tamamlandı");
    }
}

void MacroEngine::tickWarriorSerial() {
    auto& combo = m_combos["warrior_serial"];
    auto now = clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - combo.lastAction).count();
    if (elapsed < combo.config.delayMs) return;

    int step = combo.currentStep % 2;
    if (step == 0) {
        auto skill = m_config->getSkillSlot("warrior.serial_attack.skill");
        pressSkill(skill);
    } else {
        m_keys->pressR();
    }
    combo.currentStep++;
    combo.lastAction = now;
}

void MacroEngine::tickWarriorBuffs() {
    // Kafa, Kılıç, Kol, HP Booster — ikon kaybolunca yenile
    auto check = [&](const std::string& iconId, const std::string& configPath) {
        if (m_icons->needsRefresh(iconId)) {
            auto skill = m_config->getSkillSlot(configPath);
            pressSkill(skill);
            m_icons->resetRefresh(iconId);
            m_logger->info("[WARRIOR] " + iconId + " yenilendi");
        }
    };
    check("kafa",  "warrior.buff_tracking.kafa");
    check("kilic", "warrior.buff_tracking.kilic");
    check("kol",   "warrior.buff_tracking.kol");
    check("hpb",   "warrior.buff_tracking.hp_booster");
}

void MacroEngine::tickWarriorFirfir() {
    // Fırfır: hareket + skill cancel döngüsü
    // Basit implementasyon: W kısa bas → skill → W → ...
    auto& combo = m_combos["warrior_firfir"];
    auto now = clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - combo.lastAction).count();
    if (elapsed < 50) return; // 50ms aralıkla

    m_keys->pressW(30); // Çok kısa hareket
    combo.lastAction = now;
}

void MacroEngine::tickWarriorDebuff() {
    if (!m_config->getBool("warrior.debuff_cleanse.enabled")) return;

    // Buff ROI'da Torment rengi algılanırsa → kılıç sil → kalkan tak
    // Bu PixelAnalyzer'dan alınır
    auto& party = m_pixels->getPartyState();
    // Kendi debuff kontrolü (party[0] = kendimiz varsayımı)
    if (!party.empty() && party[0].debuffColor == 2) {
        // Torment algılandı
        auto shieldPos = m_config->getPosition("shield");
        if (shieldPos.recorded) {
            inventoryAction(shieldPos.x, shieldPos.y);
            m_logger->info("[WARRIOR] Torment → Kalkan takıldı");
        }
    }
}

// ══════════════════════════════════════
//  EXTRA
// ══════════════════════════════════════
void MacroEngine::tickExtraHPMP() {
    if (!m_config->getBool("extra.smart_hp_mp.enabled")) return;

    float hp = m_pixels->getHpPercent();
    float mp = m_pixels->getMpPercent();
    float hpTh = m_config->getInt("extra.smart_hp_mp.hp_threshold", 50);
    float mpTh = m_config->getInt("extra.smart_hp_mp.mp_threshold", 30);

    if (hp < hpTh && hp > 0) {
        auto pot = m_config->getSkillSlot("extra.smart_hp_mp.hp_pot");
        pressSkill(pot);
    }
    if (mp < mpTh && mp > 0) {
        auto pot = m_config->getSkillSlot("extra.smart_hp_mp.mp_pot");
        pressSkill(pot);
    }
}

void MacroEngine::tickExtraFarm() {
    // Tab → HP bar kontrol → atak → HP=0 → Tab
    // Basit state machine
    auto& combo = m_combos["extra_farm"];
    // TODO: farm state machine implementasyonu
}

void MacroEngine::tickExtraHammer() {
    if (!m_config->getBool("extra.magic_hammer.enabled")) return;
    if (m_pixels->isWeaponBroken()) {
        auto hammer = m_config->getSkillSlot("extra.magic_hammer.skill");
        pressSkill(hammer);
        m_logger->info("[EXTRA] Silah kırıldı → Hammer basıldı");
    }
}

void MacroEngine::tickExtraDuration() {
    if (!m_config->getBool("extra.duration.enabled")) return;
    // Duration skill'leri: ikon kaybolunca yenile
    // Config'den dinamik skill listesi okunur
    // Her skill için iconTracker'da izleme yapılır
    // needsRefresh → pressSkill → resetRefresh
}

std::string MacroEngine::getActiveComboName() const {
    for (auto& [name, combo] : m_combos) {
        if (combo.state == ComboState::RUNNING) return name;
    }
    return "idle";
}

} // namespace Livn

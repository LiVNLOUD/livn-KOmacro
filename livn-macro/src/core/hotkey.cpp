#include "hotkey.h"
#include "config.h"
#include "../macro/macro_engine.h"

namespace Livn {

int HotkeyManager::stringToVK(const std::string& key) {
    if (key == "CapsLock" || key == "Caps") return VK_CAPITAL;
    if (key == "F9")  return VK_F9;
    if (key == "F10") return VK_F10;
    if (key == "F11") return VK_F11;
    if (key == "F12") return VK_F12;
    if (key == "Q")   return 'Q';
    if (key == "Tab") return VK_TAB;
    if (key.size() == 1 && key[0] >= 'A' && key[0] <= 'Z') return key[0];
    return 0;
}

void HotkeyManager::poll() {
    if (!m_initialized) {
        // İlk çağrıda config'den binding'leri yükle
        auto addBinding = [&](const std::string& combo, const std::string& cfgPath) {
            auto cc = m_config->getCombo(cfgPath);
            if (!cc.shortcut.empty()) {
                HotkeyBinding hb;
                hb.comboName = combo;
                hb.vkCode = stringToVK(cc.shortcut);
                hb.mode = cc.mode;
                if (hb.vkCode != 0) m_bindings.push_back(hb);
            }
        };
        addBinding("mage_staff",     "mage.staff_combo");
        addBinding("mage_nova",      "mage.nova");
        addBinding("priest_attack",  "priest.bp_attack");
        addBinding("rogue_vs",       "rogue.vs_combo");
        addBinding("rogue_slide35",  "rogue.slide_35");
        addBinding("rogue_combo",    "rogue.skill_combo");
        addBinding("warrior_des",    "warrior.des_combo");
        addBinding("warrior_serial", "warrior.serial_attack");
        addBinding("warrior_shield", "warrior.shield_spam");
        addBinding("warrior_firfir", "warrior.firfir");
        addBinding("extra_farm",     "extra.auto_farm");
        m_initialized = true;
    }

    for (auto& b : m_bindings) {
        checkKey(b);
    }
}

void HotkeyManager::checkKey(HotkeyBinding& binding) {
    bool pressed = (GetAsyncKeyState(binding.vkCode) & 0x8000) != 0;

    if (binding.mode == "hold") {
        if (pressed && !binding.wasPressed) {
            m_engine->activateCombo(binding.comboName, true);
        } else if (!pressed && binding.wasPressed) {
            m_engine->activateCombo(binding.comboName, false);
        }
    } else { // toggle
        if (pressed && !binding.wasPressed) {
            m_engine->toggleCombo(binding.comboName);
        }
    }
    binding.wasPressed = pressed;
}

} // namespace Livn

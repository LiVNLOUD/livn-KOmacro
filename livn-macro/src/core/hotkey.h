#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include <unordered_map>

namespace Livn {

class Config;
class MacroEngine;

class HotkeyManager {
public:
    HotkeyManager(Config* config, MacroEngine* engine)
        : m_config(config), m_engine(engine) {}

    void poll();

private:
    struct HotkeyBinding {
        std::string comboName;
        int vkCode = 0;
        std::string mode = "hold";  // "hold" or "toggle"
        bool wasPressed = false;
    };

    void checkKey(HotkeyBinding& binding);
    int stringToVK(const std::string& key);

    Config* m_config;
    MacroEngine* m_engine;
    std::vector<HotkeyBinding> m_bindings;
    bool m_initialized = false;
};

} // namespace Livn

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include "config.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <Windows.h>

namespace Livn {

bool Config::load(const std::string& path) {
    m_path = path;
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "[Config] Dosya açılamadı: " << path << std::endl;
        return false;
    }
    try {
        f >> m_data;
    } catch (const json::exception& e) {
        std::cerr << "[Config] JSON parse hatası: " << e.what() << std::endl;
        return false;
    }
    return true;
}

bool Config::save(const std::string& path) {
    std::string savePath = path.empty() ? m_path : path;
    std::ofstream f(savePath);
    if (!f.is_open()) return false;
    f << m_data.dump(2);
    return true;
}

json Config::resolve(const std::string& dotPath) const {
    std::istringstream ss(dotPath);
    std::string token;
    json current = m_data;
    while (std::getline(ss, token, '.')) {
        if (current.contains(token)) {
            current = current[token];
        } else {
            return json();
        }
    }
    return current;
}

std::string Config::getString(const std::string& dotPath, const std::string& def) const {
    auto val = resolve(dotPath);
    return val.is_string() ? val.get<std::string>() : def;
}

int Config::getInt(const std::string& dotPath, int def) const {
    auto val = resolve(dotPath);
    return val.is_number() ? val.get<int>() : def;
}

bool Config::getBool(const std::string& dotPath, bool def) const {
    auto val = resolve(dotPath);
    return val.is_boolean() ? val.get<bool>() : def;
}

SkillSlot Config::getSkillSlot(const std::string& dotPath) const {
    auto val = resolve(dotPath);
    SkillSlot ss;
    if (val.is_object()) {
        ss.fTab = val.value("f_tab", "F1");
        ss.slot = val.value("slot", "0");
    }
    return ss;
}

ComboConfig Config::getCombo(const std::string& dotPath) const {
    auto val = resolve(dotPath);
    ComboConfig cc;
    if (!val.is_object()) return cc;

    cc.enabled = val.value("enabled", false);
    cc.shortcut = val.value("shortcut", "");
    cc.mode = val.value("mode", "hold");
    cc.delayMs = val.value("delay_ms", 300);

    if (val.contains("skills") && val["skills"].is_array()) {
        for (auto& s : val["skills"]) {
            SkillSlot ss;
            ss.fTab = s.value("f_tab", "F1");
            ss.slot = s.value("slot", "0");
            cc.skills.push_back(ss);
        }
    }
    return cc;
}

ROIRect Config::getROI(const std::string& name) const {
    auto val = resolve("calibration." + name);
    ROIRect r;
    if (val.is_object()) {
        r.x = val.value("x", 0);
        r.y = val.value("y", 0);
        r.w = val.value("w", 0);
        r.h = val.value("h", 0);
        r.calibrated = val.value("calibrated", false);
    }
    return r;
}

PositionRecord Config::getPosition(const std::string& name) const {
    auto val = resolve("positions." + name);
    PositionRecord p;
    if (val.is_object()) {
        p.x = val.value("x", 0);
        p.y = val.value("y", 0);
        p.recorded = val.value("recorded", false);
        p.shortcut = val.value("shortcut", "");
    }
    return p;
}

void Config::setROI(const std::string& name, const ROIRect& roi) {
    m_data["calibration"][name] = {
        {"x", roi.x}, {"y", roi.y}, {"w", roi.w}, {"h", roi.h}, {"calibrated", true}
    };
    save("");
}

void Config::setPosition(const std::string& name, int x, int y) {
    m_data["positions"][name]["x"] = x;
    m_data["positions"][name]["y"] = y;
    m_data["positions"][name]["recorded"] = true;
    save("");
}

// ── VK kod çevirme ──
int SkillSlot::getFTabVK() const {
    if (fTab == "F1") return VK_F1;
    if (fTab == "F2") return VK_F2;
    if (fTab == "F3") return VK_F3;
    if (fTab == "F4") return VK_F4;
    if (fTab == "F5") return VK_F5;
    if (fTab == "F6") return VK_F6;
    if (fTab == "F7") return VK_F7;
    if (fTab == "F8") return VK_F8;
    return VK_F1;
}

int SkillSlot::getSlotVK() const {
    if (slot.size() == 1 && slot[0] >= '0' && slot[0] <= '9') {
        return 0x30 + (slot[0] - '0'); // VK_0 to VK_9
    }
    return 0x30;
}

} // namespace Livn

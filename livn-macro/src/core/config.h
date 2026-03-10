#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace Livn {

using json = nlohmann::json;

struct SkillSlot {
    std::string fTab = "F1";  // F1-F8
    std::string slot = "0";   // 0-9

    // F tab ve slot'u gerçek VK kodlarına çevir
    int getFTabVK() const;    // VK_F1 - VK_F8
    int getSlotVK() const;    // VK_0 - VK_9 veya 0x30-0x39
};

struct ComboConfig {
    bool enabled = false;
    std::string shortcut;        // "CapsLock", "F9", "Q" vb.
    std::string mode = "hold";   // "hold" veya "toggle"
    std::vector<SkillSlot> skills;
    int delayMs = 300;           // Skill arası gecikme (USKO optimal: ~300ms)
};

struct PositionRecord {
    int x = 0, y = 0;
    bool recorded = false;
    std::string shortcut;
};

struct ROIRect {
    int x = 0, y = 0, w = 0, h = 0;
    bool calibrated = false;
};

class Config {
public:
    bool load(const std::string& path);
    bool save(const std::string& path);

    std::string getString(const std::string& dotPath, const std::string& def = "") const;
    int getInt(const std::string& dotPath, int def = 0) const;
    bool getBool(const std::string& dotPath, bool def = false) const;

    SkillSlot getSkillSlot(const std::string& dotPath) const;
    ComboConfig getCombo(const std::string& dotPath) const;
    ROIRect getROI(const std::string& name) const;
    PositionRecord getPosition(const std::string& name) const;

    // Runtime güncelleme (kalibrasyon, pozisyon kaydetme)
    void setROI(const std::string& name, const ROIRect& roi);
    void setPosition(const std::string& name, int x, int y);

    const json& raw() const { return m_data; }

private:
    json m_data;
    std::string m_path;

    // Dot-notation ile JSON erişimi: "system.target_exe" → m_data["system"]["target_exe"]
    json resolve(const std::string& dotPath) const;
};

} // namespace Livn

#pragma once
#include "config.h"

namespace Livn {

class Calibration {
public:
    void load(Config* config) { m_config = config; }
    ROIRect getROI(const std::string& name) const {
        return m_config ? m_config->getROI(name) : ROIRect{};
    }
    void setROI(const std::string& name, const ROIRect& roi) {
        if (m_config) m_config->setROI(name, roi);
    }
private:
    Config* m_config = nullptr;
};

} // namespace Livn

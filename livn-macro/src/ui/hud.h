#pragma once
#include <string>

namespace Livn {

class HudOverlay {
public:
    void update(float hp, float mp, const std::string& combo, const std::string& status) {
        // TODO: Win32 WS_EX_LAYERED transparent topmost pencere
        // Şimdilik konsola yaz
    }
};

} // namespace Livn

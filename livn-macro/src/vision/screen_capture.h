#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <opencv2/core.hpp>

namespace Livn {

class ScreenCapture {
public:
    ScreenCapture() = default;
    ~ScreenCapture();

    bool init(HWND targetWindow);
    cv::Mat capture();

private:
    HWND m_hwnd = nullptr;
    HDC m_hdcScreen = nullptr;
    HDC m_hdcMem = nullptr;
    HBITMAP m_hBitmap = nullptr;
    int m_width = 0, m_height = 0;
};

} // namespace Livn

// ── Implementation ──
// screen_capture.cpp

/*
 * BitBlt yöntemi ile ekran yakalama.
 * DXGI Desktop Duplication daha hızlı ama daha karmaşık.
 * İlk versiyon BitBlt, performans gerekirse DXGI'ya geçilir.
 *
 * Yaklaşık performans:
 *   BitBlt 1920x1080: ~8-16ms
 *   DXGI 1920x1080:   ~2-4ms
 */

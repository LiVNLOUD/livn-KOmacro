#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include "screen_capture.h"
#include <opencv2/imgproc.hpp>
#include <iostream>

namespace Livn {

ScreenCapture::~ScreenCapture() {
    if (m_hBitmap) DeleteObject(m_hBitmap);
    if (m_hdcMem)  DeleteDC(m_hdcMem);
    if (m_hdcScreen) ReleaseDC(m_hwnd, m_hdcScreen);
}

bool ScreenCapture::init(HWND targetWindow) {
    m_hwnd = targetWindow;

    // Pencere boyutunu al
    RECT rect;
    if (!GetClientRect(m_hwnd, &rect)) {
        // Pencere bulunamadı — tüm ekranı yakala
        m_hwnd = GetDesktopWindow();
        GetClientRect(m_hwnd, &rect);
    }

    m_width = rect.right - rect.left;
    m_height = rect.bottom - rect.top;

    if (m_width <= 0 || m_height <= 0) {
        std::cerr << "[ScreenCapture] Geçersiz boyut: " << m_width << "x" << m_height << std::endl;
        return false;
    }

    // DC oluştur
    m_hdcScreen = GetDC(m_hwnd);
    m_hdcMem = CreateCompatibleDC(m_hdcScreen);
    m_hBitmap = CreateCompatibleBitmap(m_hdcScreen, m_width, m_height);
    SelectObject(m_hdcMem, m_hBitmap);

    std::cout << "[ScreenCapture] Başlatıldı: " << m_width << "x" << m_height << std::endl;
    return true;
}

cv::Mat ScreenCapture::capture() {
    if (!m_hdcScreen || !m_hdcMem) return {};

    // BitBlt ile ekranı yakala
    if (!BitBlt(m_hdcMem, 0, 0, m_width, m_height, m_hdcScreen, 0, 0, SRCCOPY)) {
        return {};
    }

    // HBITMAP → cv::Mat dönüşümü
    BITMAPINFOHEADER bi = {};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = m_width;
    bi.biHeight = -m_height;  // Negatif = top-down
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;

    cv::Mat frame(m_height, m_width, CV_8UC4);
    GetDIBits(m_hdcMem, m_hBitmap, 0, m_height, frame.data,
              reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS);

    // BGRA → BGR
    cv::Mat bgr;
    cv::cvtColor(frame, bgr, cv::COLOR_BGRA2BGR);
    return bgr;
}

} // namespace Livn

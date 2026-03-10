#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include "roi_manager.h"
#include "../core/calibration.h"
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <iostream>

namespace Livn {

// Renk tablosu — debug annotasyon için
static const std::unordered_map<std::string, cv::Scalar> DEBUG_COLORS = {
    { "hp_bar",       cv::Scalar(0,   80,  220) },  // Kırmızı
    { "mp_bar",       cv::Scalar(200, 80,  0  ) },  // Mavi
    { "party_bar",    cv::Scalar(0,   180, 80 ) },  // Yeşil
    { "buff_bar",     cv::Scalar(200, 180, 0  ) },  // Sarı
    { "skill_bar",    cv::Scalar(220, 120, 0  ) },  // Cyan
    { "chat_area",    cv::Scalar(180, 0,   180) },  // Mor
    { "weapon_break", cv::Scalar(0,   180, 220) },  // Turuncu
};

static const std::vector<std::string> ALL_ROI_NAMES = {
    "hp_bar", "mp_bar", "party_bar", "buff_bar",
    "skill_bar", "chat_area", "weapon_break"
};

RoiManager::RoiManager(Calibration* calib) : m_calib(calib) {}

void RoiManager::updateFrame(const cv::Mat& frame) {
    std::lock_guard<std::mutex> lock(m_frameMutex);
    // Kopyalamak yerine referans say — performans için
    m_frame = frame;
}

cv::Rect RoiManager::clampRect(const cv::Rect& r, int w, int h) const {
    int x = std::max(0, r.x);
    int y = std::max(0, r.y);
    int x2 = std::min(w, r.x + r.width);
    int y2 = std::min(h, r.y + r.height);
    int cw = x2 - x;
    int ch = y2 - y;
    if (cw <= 0 || ch <= 0) return cv::Rect(); // Geçersiz
    return cv::Rect(x, y, cw, ch);
}

cv::Mat RoiManager::getROI(const std::string& name) const {
    std::lock_guard<std::mutex> lock(m_frameMutex);

    if (m_frame.empty()) {
        if (m_debugMode)
            std::cerr << "[ROI:" << name << "] Frame boş — vision başlamadı\n";
        return {};
    }

    if (!m_calib) {
        if (m_debugMode)
            std::cerr << "[ROI:" << name << "] Calibration null\n";
        return {};
    }

    ROIRect roi = m_calib->getROI(name);

    if (!roi.calibrated) {
        if (m_debugMode)
            std::cout << "[ROI:" << name << "] Henüz kalibre edilmemiş — atlanıyor\n";
        return {};
    }

    cv::Rect rect(roi.x, roi.y, roi.w, roi.h);
    cv::Rect safe = clampRect(rect, m_frame.cols, m_frame.rows);

    if (safe.empty()) {
        if (m_debugMode)
            std::cerr << "[ROI:" << name << "] Rect frame dışında: "
                      << roi.x << "," << roi.y << " " << roi.w << "x" << roi.h
                      << " (frame: " << m_frame.cols << "x" << m_frame.rows << ")\n";
        return {};
    }

    if (m_debugMode) {
        std::cout << "[ROI:" << name << "] Kesildi: "
                  << safe.x << "," << safe.y << " " << safe.width << "x" << safe.height << "\n";
    }

    // Frame kopyası değil, sub-matrix (sıfır kopya)
    return m_frame(safe);
}

int RoiManager::frameWidth()  const {
    std::lock_guard<std::mutex> lock(m_frameMutex);
    return m_frame.cols;
}

int RoiManager::frameHeight() const {
    std::lock_guard<std::mutex> lock(m_frameMutex);
    return m_frame.rows;
}

cv::Mat RoiManager::getAnnotatedFrame() const {
    std::lock_guard<std::mutex> lock(m_frameMutex);

    if (m_frame.empty()) return {};

    cv::Mat annotated = m_frame.clone();

    for (const auto& name : ALL_ROI_NAMES) {
        ROIRect roi = m_calib->getROI(name);
        if (!roi.calibrated) continue;

        cv::Rect rect(roi.x, roi.y, roi.w, roi.h);
        cv::Rect safe = clampRect(rect, annotated.cols, annotated.rows);
        if (safe.empty()) continue;

        // Renk seç
        auto it = DEBUG_COLORS.find(name);
        cv::Scalar color = (it != DEBUG_COLORS.end())
            ? it->second
            : cv::Scalar(255, 255, 255);

        // Dikdörtgen çiz
        cv::rectangle(annotated, safe, color, 2);

        // İsim etiketi
        cv::putText(annotated, name,
                    cv::Point(safe.x + 3, safe.y + 14),
                    cv::FONT_HERSHEY_SIMPLEX, 0.45, color, 1);
    }

    return annotated;
}

} // namespace Livn

#pragma once
#include <opencv2/core.hpp>
#include <string>
#include <unordered_map>
#include <mutex>

namespace Livn {

class Calibration;

/*
 * RoiManager — Frame'den kalibre edilmiş bölgeleri keser.
 *
 * Kullanım:
 *   1. Her vision tick'te updateFrame() ile güncel frame verilir.
 *   2. getROI("hp_bar") çağrısı kalibrasyon rect'ini okur, frame'den keser.
 *   3. Kalibrasyon yapılmamışsa (calibrated=false) boş Mat döner.
 *
 * Debug modu:
 *   setDebugMode(true) → getROI() her çağrıda debug bilgisi loglar.
 *   getAnnotatedFrame() → tüm ROI bölgelerini renkli dikdörtgen olarak
 *   overlay'ler — kalibrasyon doğrulaması için kullanılır.
 */
class RoiManager {
public:
    explicit RoiManager(Calibration* calib);

    // Her vision frame'i buraya ver
    void updateFrame(const cv::Mat& frame);

    // İsme göre ROI kes. Kalibrasyon yoksa boş Mat döner.
    cv::Mat getROI(const std::string& name) const;

    // Frame boyutlarını döndür (kalibrasyon UI için)
    int frameWidth()  const;
    int frameHeight() const;

    // Debug: tüm kalibre ROI'ları overlay'li frame döndür
    cv::Mat getAnnotatedFrame() const;

    // Debug modu (konsol log)
    void setDebugMode(bool on) { m_debugMode = on; }

private:
    Calibration*      m_calib;
    cv::Mat           m_frame;
    mutable std::mutex m_frameMutex;
    bool              m_debugMode = false;

    // Güvenli kırpma: rect frame sınırları dışına çıkıyorsa klemplenır
    cv::Rect clampRect(const cv::Rect& r, int w, int h) const;
};

} // namespace Livn

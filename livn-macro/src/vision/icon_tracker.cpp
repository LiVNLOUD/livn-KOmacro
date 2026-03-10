#include "icon_tracker.h"
#include "pixel_analyzer.h"
#include "roi_manager.h"
#include <opencv2/imgproc.hpp>
#include <stdexcept>

namespace Livn {

IconTracker::IconTracker(PixelAnalyzer* analyzer, RoiManager* roi)
    : m_analyzer(analyzer), m_roi(roi) {}

void IconTracker::autoSetupIcon(const std::string& iconId,
                                const std::string& fTab,
                                const std::string& slot,
                                const cv::Rect& barRect,
                                int barCols)
{
    /*
     * KO Skill Bar Layout:
     *
     * Skill bar sabit genişliktedir. Her slot eşit genişlikte bir hücre.
     * Slot numarası (0-9) → hücrenin x offset'i.
     *
     * fTab (F1-F8) → bar'ın hangi sekme satırı olduğunu belirtir.
     * Burada tek bir bar ROI verildiği varsayılır (tek satır).
     * Çoklu satır için barRect farklı çağrıyla verilir.
     */
    if (barRect.empty() || barCols <= 0) return;

    int slotIndex = 0;
    try { slotIndex = std::stoi(slot); } catch (...) {}
    slotIndex = std::max(0, std::min(slotIndex, barCols - 1));

    int cellW = barRect.width  / barCols;
    int cellH = barRect.height;

    // Küçük bir iç marj bırak (kenar gölgelerini atla)
    int margin = 2;
    cv::Rect iconRect(
        barRect.x + slotIndex * cellW + margin,
        barRect.y + margin,
        cellW - margin * 2,
        cellH - margin * 2
    );

    // RoiManager koordinatları bar'ın içine göre değil — barRect zaten
    // ekran koordinatı. TrackIcon'a göreceli rect veriyoruz (ROI içi).
    cv::Rect relRect(
        slotIndex * cellW + margin,
        margin,
        cellW - margin * 2,
        cellH - margin * 2
    );

    trackIcon(iconId, relRect);
}

void IconTracker::trackIcon(const std::string& id, const cv::Rect& region) {
    TrackedIcon icon;
    icon.id = id;
    icon.region = region;
    icon.lastChange = std::chrono::steady_clock::now();
    m_icons[id] = icon;
}

float IconTracker::measureBrightness(const cv::Mat& iconROI) {
    if (iconROI.empty()) return 0.0f;

    cv::Mat hsv;
    cv::cvtColor(iconROI, hsv, cv::COLOR_BGR2HSV);

    // V (Value) kanalının ortalaması = parlaklık
    std::vector<cv::Mat> channels;
    cv::split(hsv, channels);

    return static_cast<float>(cv::mean(channels[2])[0]);
}

void IconTracker::updateIconState(TrackedIcon& icon, float brightness) {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - icon.lastChange).count();

    float delta = std::abs(brightness - icon.prevBrightness);

    // Durum geçiş mantığı
    if (brightness < BRIGHTNESS_THRESHOLD) {
        // İkon kayboldu
        if (icon.state != IconState::GONE) {
            icon.state = IconState::GONE;
            icon.needsRefresh = true;
            icon.blinkCount = 0;
            icon.lastChange = now;
        }
    } else if (delta > BLINK_DELTA && elapsed < 800) {
        // Hızlı parlaklık değişimi = yanıp sönme
        icon.blinkCount++;
        if (icon.blinkCount >= BLINK_COUNT_THRESHOLD) {
            icon.state = IconState::BLINKING;
            // Bitmek üzere — henüz yenileme tetikleme, biraz bekle
        }
        icon.lastChange = now;
    } else if (brightness > 150.0f && delta < 5.0f) {
        // Yüksek ve sabit parlaklık = hazır
        if (icon.state == IconState::ACTIVE || icon.state == IconState::BLINKING) {
            // CD'den READY'ye geçiş = CD bitti
            icon.state = IconState::READY;
            icon.needsRefresh = true;
            icon.blinkCount = 0;
        } else {
            icon.state = IconState::READY;
        }
        icon.lastChange = now;
    } else {
        // Normal parlaklık = aktif (CD devam veya buff aktif)
        if (icon.state == IconState::UNKNOWN || icon.state == IconState::GONE) {
            icon.state = IconState::ACTIVE;
        }
        if (elapsed > 1000) {
            icon.blinkCount = 0; // Uzun süredir değişim yok = blink sıfırla
        }
    }

    icon.prevBrightness = icon.lastBrightness;
    icon.lastBrightness = brightness;
}

void IconTracker::update(const cv::Mat& buffBarROI) {
    if (buffBarROI.empty()) return;

    for (auto& [id, icon] : m_icons) {
        // Sadece buff bar'daki ikonları güncelle
        if (icon.region.x < 0 || icon.region.y < 0) continue;
        if (icon.region.x + icon.region.width > buffBarROI.cols) continue;
        if (icon.region.y + icon.region.height > buffBarROI.rows) continue;

        cv::Mat iconROI = buffBarROI(icon.region);
        float brightness = measureBrightness(iconROI);
        updateIconState(icon, brightness);
    }
}

void IconTracker::updateSkillBar(const cv::Mat& skillBarROI) {
    // Skill bar CD takibi — aynı mantık
    // Kitap, Kol gibi CD ikonları burada izlenir
    update(skillBarROI); // Aynı algoritmayı kullanır
}

IconState IconTracker::getIconState(const std::string& id) const {
    auto it = m_icons.find(id);
    return it != m_icons.end() ? it->second.state : IconState::UNKNOWN;
}

bool IconTracker::needsRefresh(const std::string& id) const {
    auto it = m_icons.find(id);
    return it != m_icons.end() && it->second.needsRefresh;
}

void IconTracker::resetRefresh(const std::string& id) {
    auto it = m_icons.find(id);
    if (it != m_icons.end()) it->second.needsRefresh = false;
}

std::string IconTracker::getStatus() const {
    std::string status;
    for (const auto& [id, icon] : m_icons) {
        status += id + ":";
        switch (icon.state) {
            case IconState::ACTIVE:   status += "ACT "; break;
            case IconState::BLINKING: status += "BLN "; break;
            case IconState::GONE:     status += "OFF "; break;
            case IconState::READY:    status += "RDY "; break;
            default:                  status += "??? "; break;
        }
    }
    return status;
}

} // namespace Livn

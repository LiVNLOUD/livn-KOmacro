#include "pixel_analyzer.h"
#include <opencv2/imgproc.hpp>
#include <algorithm>

namespace Livn {

float PixelAnalyzer::calculateBarPercent(const cv::Mat& roi, cv::Scalar low, cv::Scalar high) {
    if (roi.empty()) return 0.0f;

    cv::Mat hsv;
    cv::cvtColor(roi, hsv, cv::COLOR_BGR2HSV);

    cv::Mat mask;
    cv::inRange(hsv, low, high, mask);

    int coloredPixels = cv::countNonZero(mask);
    int totalPixels = roi.cols * roi.rows;

    if (totalPixels == 0) return 0.0f;
    return (static_cast<float>(coloredPixels) / totalPixels) * 100.0f;
}

/*
 * Bar doluluk oranı hesaplama — Pixel sayısı yerine sütun tarama yöntemi.
 *
 * KO'da HP/MP barları soldan dolar. En sağdaki renkli sütunun x pozisyonu
 * / bar genişliği = doluluk yüzdesi. Bu yöntem renk yoğunluğuna bağımlı
 * değil, kenar bölgesindeki gölge/parlaklık varyasyonlarından etkilenmiyor.
 */
float PixelAnalyzer::calculateBarFillRatio(const cv::Mat& roi,
                                            cv::Scalar low, cv::Scalar high) {
    if (roi.empty()) return 0.0f;

    cv::Mat hsv;
    cv::cvtColor(roi, hsv, cv::COLOR_BGR2HSV);

    cv::Mat mask;
    cv::inRange(hsv, low, high, mask);

    // Her sütunda renkli pixel var mı kontrol et (soldan sağa)
    int lastFilledCol = -1;
    for (int col = 0; col < mask.cols; col++) {
        cv::Mat colSlice = mask.col(col);
        if (cv::countNonZero(colSlice) > mask.rows * 0.2f) { // %20 eşik
            lastFilledCol = col;
        }
    }

    if (lastFilledCol < 0) return 0.0f;
    return (static_cast<float>(lastFilledCol + 1) / mask.cols) * 100.0f;
}

void PixelAnalyzer::analyzeHpBar(const cv::Mat& roi) {
    if (roi.empty()) return;

    // KO HP bar: kırmızı (H=0-10 ve H=160-180)
    float pct1 = calculateBarFillRatio(roi,
        cv::Scalar(0,   100, 80), cv::Scalar(10,  255, 255));
    float pct2 = calculateBarFillRatio(roi,
        cv::Scalar(160, 100, 80), cv::Scalar(180, 255, 255));

    float pct = std::max(pct1, pct2); // İki kırmızı aralığından büyüğü

    std::lock_guard<std::mutex> lock(m_mutex);
    m_hp.percent = pct;
    m_hp.valid   = true;
}

void PixelAnalyzer::analyzeMpBar(const cv::Mat& roi) {
    if (roi.empty()) return;

    // KO MP bar: mavi (H=100-130)
    float pct = calculateBarFillRatio(roi,
        cv::Scalar(100, 100, 80), cv::Scalar(130, 255, 255));

    std::lock_guard<std::mutex> lock(m_mutex);
    m_mp.percent = pct;
    m_mp.valid   = true;
}

void PixelAnalyzer::analyzePartyBars(const cv::Mat& roi) {
    if (roi.empty()) return;

    /*
     * Parti bar yapısı (KO):
     * - 8 üyeye kadar, her biri dikey olarak eşit yükseklikte bir şerit
     * - Her şeritte: isim alanı (sol) + HP bar (sağ kısım)
     * - HP bar: yeşil (yüksek HP) → sarı → kırmızı (düşük HP)
     *
     * Yaklaşım: Her satır şeridinde, HP renkleri (yeşil/sarı/kırmızı)
     * içeren sütun sayısı / toplam bar genişliği = HP yüzdesi.
     * Bar genişliği: şeridin sağ %60'ı (isim alanını atla).
     */
    int memberCount = 8;
    int memberH = roi.rows / memberCount;
    if (memberH < 3) return;

    // Parti bar'ında HP bölgesi yaklaşık sağ %65'tir
    int barStartX = static_cast<int>(roi.cols * 0.35f);
    int barW      = roi.cols - barStartX;
    if (barW <= 0) return;

    std::vector<PartyMember> newParty;

    for (int i = 0; i < memberCount; i++) {
        int y = i * memberH;
        if (y + memberH > roi.rows) break;

        cv::Rect memberRect(barStartX, y, barW, memberH);
        cv::Mat memberROI = roi(memberRect);

        PartyMember pm;

        // Yeşil HP (yüksek)
        float green = calculateBarFillRatio(memberROI,
            cv::Scalar(35, 80, 80), cv::Scalar(85, 255, 255));
        // Sarı HP (orta)
        float yellow = calculateBarFillRatio(memberROI,
            cv::Scalar(20, 80, 80), cv::Scalar(35, 255, 255));
        // Kırmızı HP (düşük)
        float red1 = calculateBarFillRatio(memberROI,
            cv::Scalar(0,   100, 80), cv::Scalar(10,  255, 255));
        float red2 = calculateBarFillRatio(memberROI,
            cv::Scalar(160, 100, 80), cv::Scalar(180, 255, 255));
        float red  = std::max(red1, red2);

        // En yüksek doluluk oranı = HP yüzdesi
        pm.hpPercent = std::max({ green, yellow, red });

        // Debuff tespiti: tüm üye satırına bak (isim alanı dahil)
        cv::Mat fullRow = roi(cv::Rect(0, y, roi.cols, memberH));
        pm.debuffColor  = detectDebuffColor(fullRow);
        pm.hasDebuff    = pm.debuffColor > 0;

        newParty.push_back(pm);
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    m_party = newParty;
}

void PixelAnalyzer::checkWeaponBreak(const cv::Mat& roi) {
    if (roi.empty()) return;

    // Silah kırılma ikonu: sarı/turuncu uyarı rengi
    float warn = calculateBarPercent(roi,
        cv::Scalar(15, 150, 150), cv::Scalar(35, 255, 255));

    std::lock_guard<std::mutex> lock(m_mutex);
    m_weaponBroken = (warn > 15.0f);
}

int PixelAnalyzer::detectDebuffColor(const cv::Mat& roi) {
    if (roi.empty()) return 0;

    cv::Mat hsv;
    cv::cvtColor(roi, hsv, cv::COLOR_BGR2HSV);
    float threshold = static_cast<float>(roi.total()) * 0.04f;

    // Malice: mor (H=120-160)
    cv::Mat mask;
    cv::inRange(hsv, cv::Scalar(120, 70, 70), cv::Scalar(160, 255, 255), mask);
    if (cv::countNonZero(mask) > threshold) return 1;

    // Torment: kırmızı (iki aralık)
    cv::Mat m1, m2;
    cv::inRange(hsv, cv::Scalar(0,   120, 100), cv::Scalar(8,   255, 255), m1);
    cv::inRange(hsv, cv::Scalar(170, 120, 100), cv::Scalar(180, 255, 255), m2);
    if (cv::countNonZero(m1) + cv::countNonZero(m2) > threshold) return 2;

    // Poison: yeşil (H=35-85)
    cv::inRange(hsv, cv::Scalar(35, 100, 80), cv::Scalar(85, 255, 255), mask);
    if (cv::countNonZero(mask) > threshold) return 3;

    // Slow: mavi (H=90-130)
    cv::inRange(hsv, cv::Scalar(90, 100, 80), cv::Scalar(130, 255, 255), mask);
    if (cv::countNonZero(mask) > threshold) return 4;

    return 0;
}

float PixelAnalyzer::getHpPercent() const {
    std::lock_guard<std::mutex> l(m_mutex);
    return m_hp.percent;
}
float PixelAnalyzer::getMpPercent() const {
    std::lock_guard<std::mutex> l(m_mutex);
    return m_mp.percent;
}
const std::vector<PartyMember>& PixelAnalyzer::getPartyState() const {
    // NOT: Çağıran mutex almadan okuyabilir — kısa window kabul edilir.
    // Kritik kullanımda ayrı lock gerekir.
    return m_party;
}
bool PixelAnalyzer::isWeaponBroken() const {
    std::lock_guard<std::mutex> l(m_mutex);
    return m_weaponBroken;
}

} // namespace Livn

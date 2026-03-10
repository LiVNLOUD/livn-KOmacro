#pragma once
#include <opencv2/core.hpp>
#include <mutex>
#include <vector>

namespace Livn {

struct BarState {
    float percent = 100.0f;
    bool valid = false;
};

struct PartyMember {
    float hpPercent = 100.0f;
    bool hasDebuff = false;
    int debuffColor = 0; // 0=none, 1=malice(purple), 2=torment(red), 3=poison(green), 4=slow(blue)
};

class PixelAnalyzer {
public:
    // HP/MP bar analizi: kırmızı/mavi pixel oranı ölçümü
    void analyzeHpBar(const cv::Mat& roi);
    void analyzeMpBar(const cv::Mat& roi);

    // Parti HP barları — her üye için HP yüzdesi
    void analyzePartyBars(const cv::Mat& roi);

    // Silah kırılma ikonu tespiti
    void checkWeaponBreak(const cv::Mat& roi);

    // Debuff renk imzası algılama
    int detectDebuffColor(const cv::Mat& roi);

    // Sonuçları oku (thread-safe)
    float getHpPercent() const;
    float getMpPercent() const;
    const std::vector<PartyMember>& getPartyState() const;
    bool isWeaponBroken() const;

private:
    // Bar pixel oranı hesaplama
    // Kırmızı piksel sayısı / toplam bar genişliği = HP yüzdesi
    float calculateBarPercent(const cv::Mat& roi, cv::Scalar lowColor, cv::Scalar highColor);

    // Sütun tarama ile bar doluluk oranı (soldan sağa fill ratio)
    // Pixel yoğunluğundan bağımsız, KO bar'ları için daha doğru
    float calculateBarFillRatio(const cv::Mat& roi, cv::Scalar low, cv::Scalar high);

    mutable std::mutex m_mutex;
    BarState m_hp, m_mp;
    std::vector<PartyMember> m_party;
    bool m_weaponBroken = false;
};

} // namespace Livn

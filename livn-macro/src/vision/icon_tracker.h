#pragma once
#include <opencv2/core.hpp>
#include <unordered_map>
#include <string>
#include <chrono>

namespace Livn {

class PixelAnalyzer;
class RoiManager;
class Config;

enum class IconState {
    UNKNOWN,       // Henüz izlenmiyor
    ACTIVE,        // İkon görünür, parlaklık normal = aktif/CD devam
    BLINKING,      // Yanıp sönüyor = bitmek üzere (~500ms aralık)
    GONE,          // Kayboldu = bitti / CD tamamlandı → yenilenmeli
    READY          // Skill kullanıma hazır (ikon parlak)
};

struct TrackedIcon {
    std::string id;          // "kitap", "kol", "echo", "kafa", "lf" vb.
    cv::Rect region;         // Skill bar üzerindeki piksel bölgesi
    IconState state = IconState::UNKNOWN;
    float lastBrightness = 0;
    float prevBrightness = 0;
    int blinkCount = 0;
    std::chrono::steady_clock::time_point lastChange;
    bool needsRefresh = false;
};

class IconTracker {
public:
    IconTracker(PixelAnalyzer* analyzer, RoiManager* roi);

    /*
     * Skill bar üzerindeki ikon bölgelerini config'den otomatik hesapla.
     *
     * KO skill bar: F1-F8 arasında sekiz sekme, her sekmede 0-9 slot.
     * skillBarROI içinde slot pozisyonları eşit aralıklıdır.
     *
     * Parametreler:
     *   cfg        — Config nesnesi (f_tab / slot okumak için)
     *   iconId     — Takip edilecek isim ("kitap", "kol" vb.)
     *   configPath — Config'deki skill slot yolu ("priest.bp_attack.kitap")
     *   barRect    — Kalibre edilmiş skill_bar ROI'si (ekran koordinatları)
     *   barCols    — Bar'daki toplam slot sayısı (varsayılan: 10)
     */
    void autoSetupIcon(const std::string& iconId,
                       const std::string& fTab,
                       const std::string& slot,
                       const cv::Rect& barRect,
                       int barCols = 10);

    // Buff/debuff bar üzerindeki ikonları güncelle
    void update(const cv::Mat& buffBarROI);

    // Skill bar üzerindeki CD ikonlarını güncelle
    void updateSkillBar(const cv::Mat& skillBarROI);

    // İkon durumu sorgula
    IconState getIconState(const std::string& id) const;
    bool needsRefresh(const std::string& id) const;
    void resetRefresh(const std::string& id);

    // Belirli bir SkillSlot için ikon bölgesini kaydet
    void trackIcon(const std::string& id, const cv::Rect& region);

    std::string getStatus() const;

private:
    /*
     * İkon parlaklık tespiti:
     * 1. ROI bölgesinin ortalama parlaklığı hesaplanır (V kanalı)
     * 2. Önceki frame ile karşılaştırılır
     * 3. Büyük değişim = durum geçişi:
     *    - Parlaklık düştü → ACTIVE (CD devam)
     *    - Parlaklık salınım yapıyor (~500ms) → BLINKING (bitmek üzere)
     *    - Parlaklık = 0 veya çok düşük → GONE (bitti)
     *    - Parlaklık yüksek ve sabit → READY (kullanılabilir)
     */
    float measureBrightness(const cv::Mat& iconROI);
    void updateIconState(TrackedIcon& icon, float brightness);

    PixelAnalyzer* m_analyzer;
    RoiManager*    m_roi;
    std::unordered_map<std::string, TrackedIcon> m_icons;

    static constexpr float BRIGHTNESS_THRESHOLD = 30.0f;  // Gone eşiği
    static constexpr float BLINK_DELTA          = 20.0f;  // Yanıp sönme eşiği
    static constexpr int   BLINK_COUNT_THRESHOLD = 3;     // Kaç kez yanıp sönerse BLINKING
};

} // namespace Livn

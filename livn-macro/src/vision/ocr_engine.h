#pragma once
#include <opencv2/core.hpp>
#include <string>

namespace Livn {

class OcrEngine {
public:
    bool init(const std::string& lang) {
        // TODO: Tesseract API başlatma
        // tesseract::TessBaseAPI kullanılacak
        m_ready = false; // Şimdilik devre dışı
        return true;
    }
    bool isReady() const { return m_ready; }
    void processChat(const cv::Mat& roi) {
        // TODO: Chat alanını OCR ile oku, "tp" kelimesini ara
    }
    std::string readText(const cv::Mat& roi) {
        // TODO: Verilen ROI'daki metni oku
        return "";
    }
private:
    bool m_ready = false;
};

} // namespace Livn

/*
 * Livn Macro v4.1 — Knight Online USKO
 * Ekran okuma tabanlı macro sistemi
 * Inject YOK — oyun hafızasına/dosyalarına dokunmaz
 *
 * Mimari:
 *   main.cpp          → Uygulama başlatma, config yükleme, thread'leri başlatma
 *   core/             → Config, kalibrasyon, hotkey dinleme, loglama
 *   vision/           → Ekran yakalama, pixel analiz, OCR, ikon takip
 *   input/            → Interception driver, tuş/mouse gönderme, humanizer
 *   macro/            → Macro engine, combo runner, CD tracker, sınıf modülleri
 *   ui/               → HUD overlay penceresi
 */

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "core/app.h"
#include <iostream>

int main(int argc, char* argv[]) {
    // Konsol UTF-8 ve Türkçe karakter desteği
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    std::cout << "========================================" << std::endl;
    std::cout << "  LIVN MACRO v4.1 — Knight Online USKO" << std::endl;
    std::cout << "  No Inject | Ekran Okuma | Driver" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    Livn::App app;

    if (!app.init("config.json")) {
        std::cout << std::endl;
        std::cout << "[HATA] Baslatma basarisiz." << std::endl;
        std::cout << "Kapatmak icin bir tusa basin..." << std::endl;
        std::cin.get();
        return 1;
    }

    app.run();

    std::cout << std::endl;
    std::cout << "Kapatmak icin bir tusa basin..." << std::endl;
    std::cin.get();
    return 0;
}

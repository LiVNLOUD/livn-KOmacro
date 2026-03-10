#pragma once

#define NOMINMAX           // Windows.h min/max macro'larını devre dışı bırak (OpenCV çakışması)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <string>
#include <memory>
#include <atomic>
#include <thread>

namespace Livn {

// Forward declarations
class Config;
class Calibration;
class HotkeyManager;
class Logger;
class ScreenCapture;
class PixelAnalyzer;
class OcrEngine;
class RoiManager;
class IconTracker;
class InterceptionDriver;
class KeySender;
class MouseSender;
class Humanizer;
class MacroEngine;
class HudOverlay;

class App {
public:
    App();
    ~App();

    bool init(const std::string& configPath);
    void run();
    void shutdown();

private:
    void visionLoop();    // Ekran yakalama + analiz thread
    void macroLoop();     // Macro karar + uygulama thread
    void hotkeyLoop();    // Hotkey dinleme thread
    void hudLoop();       // HUD overlay thread

    // Modüller
    std::unique_ptr<Config>             m_config;
    std::unique_ptr<Calibration>        m_calibration;
    std::unique_ptr<HotkeyManager>      m_hotkeys;
    std::unique_ptr<Logger>             m_logger;
    std::unique_ptr<ScreenCapture>      m_screenCap;
    std::unique_ptr<PixelAnalyzer>      m_pixelAnalyzer;
    std::unique_ptr<OcrEngine>          m_ocr;
    std::unique_ptr<RoiManager>         m_roiManager;
    std::unique_ptr<IconTracker>        m_iconTracker;
    std::unique_ptr<InterceptionDriver> m_driver;
    std::unique_ptr<KeySender>          m_keySender;
    std::unique_ptr<MouseSender>        m_mouseSender;
    std::unique_ptr<Humanizer>          m_humanizer;
    std::unique_ptr<MacroEngine>        m_macroEngine;
    std::unique_ptr<HudOverlay>         m_hud;

    // Thread'ler
    std::thread m_visionThread;
    std::thread m_macroThread;
    std::thread m_hotkeyThread;
    std::thread m_hudThread;

    std::atomic<bool> m_running{false};
    HWND m_targetWindow{nullptr};
};

} // namespace Livn

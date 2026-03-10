#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "interception.h"
#include <random>
#include <thread>
#include <chrono>
#include <iostream>

namespace Livn {

// ── Humanizer ──
class Humanizer {
public:
    Humanizer(int jitterMs = 5, bool enabled = true)
        : m_jitter(jitterMs), m_enabled(enabled), m_rng(std::random_device{}()) {}

    int getDelay(int baseMs) {
        if (!m_enabled || m_jitter == 0) return baseMs;
        std::uniform_int_distribution<int> d(-m_jitter, m_jitter);
        return (std::max)(1, baseMs + d(m_rng));
    }
    int getKeyHoldTime() {
        std::uniform_int_distribution<int> d(30, 80);
        return d(m_rng);
    }
    void wait(int baseMs) {
        int ms = getDelay(baseMs);
        if (ms > 0) std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
private:
    int m_jitter;
    bool m_enabled;
    std::mt19937 m_rng;
};

// ── Interception Driver ──
class InterceptionDriver {
public:
    bool init() {
        m_context = interception_create_context();
        if (!m_context) {
            std::cerr << "[Interception] Context olusturulamadi" << std::endl;
            return false;
        }

        // ÖNEMLİ: interception_set_filter ÇAĞIRMA!
        // Filter ayarlamak tüm klavye girdilerini yakalar ve geri iletmezse klavye kilitlenir.
        // Biz sadece gönderme (send) yapacağız, yakalama (receive) yapmayacağız.

        // Klavye device: 1-10 arası. Ana klavye genelde 1.
        m_keyboardDevice = 1;

        m_ready = true;
        std::cout << "[Interception] Driver aktif — HW level input (sadece gonderme)" << std::endl;
        return true;
    }

    ~InterceptionDriver() {
        if (m_context) {
            interception_destroy_context(m_context);
        }
    }

    bool isReady() const { return m_ready; }

    void sendKeyDown(unsigned short scanCode) {
        if (!m_ready) return;
        InterceptionKeyStroke stroke = {};
        stroke.code = scanCode;
        stroke.state = INTERCEPTION_KEY_DOWN;
        interception_send(m_context, m_keyboardDevice,
            reinterpret_cast<InterceptionStroke*>(&stroke), 1);
    }

    void sendKeyUp(unsigned short scanCode) {
        if (!m_ready) return;
        InterceptionKeyStroke stroke = {};
        stroke.code = scanCode;
        stroke.state = INTERCEPTION_KEY_UP;
        interception_send(m_context, m_keyboardDevice,
            reinterpret_cast<InterceptionStroke*>(&stroke), 1);
    }

    void sendKeyPress(unsigned short scanCode, int holdMs = 50) {
        sendKeyDown(scanCode);
        std::this_thread::sleep_for(std::chrono::milliseconds(holdMs));
        sendKeyUp(scanCode);
    }

private:
    InterceptionContext m_context = nullptr;
    InterceptionDevice m_keyboardDevice = 1;
    bool m_ready = false;
};

// ── Key Sender ──
class KeySender {
public:
    KeySender(InterceptionDriver* driver, Humanizer* humanizer)
        : m_driver(driver), m_humanizer(humanizer) {}

    void pressSkillSlot(int fTabVK, int slotVK) {
        if (m_currentFTab != fTabVK) {
            pressKey(fTabVK, 30);
            m_humanizer->wait(50);
            m_currentFTab = fTabVK;
        }
        pressKey(slotVK, m_humanizer->getKeyHoldTime());
    }

    void pressKey(int vk, int holdMs = 50) {
        UINT scan = MapVirtualKeyA(vk, MAPVK_VK_TO_VSC);

        if (m_driver && m_driver->isReady()) {
            // Interception driver ile — donanımsal seviye
            m_driver->sendKeyPress(static_cast<unsigned short>(scan), holdMs);
        } else {
            // SendInput fallback — scan code ile
            INPUT inp[2] = {};
            inp[0].type = INPUT_KEYBOARD;
            inp[0].ki.wVk = static_cast<WORD>(vk);
            inp[0].ki.wScan = static_cast<WORD>(scan);
            inp[0].ki.dwFlags = KEYEVENTF_SCANCODE;
            SendInput(1, &inp[0], sizeof(INPUT));

            std::this_thread::sleep_for(std::chrono::milliseconds(holdMs));

            inp[1].type = INPUT_KEYBOARD;
            inp[1].ki.wVk = static_cast<WORD>(vk);
            inp[1].ki.wScan = static_cast<WORD>(scan);
            inp[1].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
            SendInput(1, &inp[1], sizeof(INPUT));
        }
    }

    void pressR() { pressKey('R', m_humanizer->getKeyHoldTime()); }
    void pressZ() { pressKey('Z', m_humanizer->getKeyHoldTime()); }

    void pressW(int ms) {
        UINT scan = MapVirtualKeyA('W', MAPVK_VK_TO_VSC);
        if (m_driver && m_driver->isReady()) {
            m_driver->sendKeyDown(static_cast<unsigned short>(scan));
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
            m_driver->sendKeyUp(static_cast<unsigned short>(scan));
        } else {
            INPUT inp = {};
            inp.type = INPUT_KEYBOARD;
            inp.ki.wScan = static_cast<WORD>(scan);
            inp.ki.dwFlags = KEYEVENTF_SCANCODE;
            SendInput(1, &inp, sizeof(INPUT));
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
            inp.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
            SendInput(1, &inp, sizeof(INPUT));
        }
    }

private:
    InterceptionDriver* m_driver;
    Humanizer* m_humanizer;
    int m_currentFTab = -1;
};

// ── Mouse Sender ──
class MouseSender {
public:
    MouseSender(InterceptionDriver* driver, Humanizer* humanizer)
        : m_driver(driver), m_humanizer(humanizer) {}
    void clickAt(int x, int y) {
        SetCursorPos(x, y);
        m_humanizer->wait(30);
        INPUT inp = {};
        inp.type = INPUT_MOUSE;
        inp.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        SendInput(1, &inp, sizeof(INPUT));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        inp.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SendInput(1, &inp, sizeof(INPUT));
    }
    void rightClickAt(int x, int y) {
        SetCursorPos(x, y);
        m_humanizer->wait(30);
        INPUT inp = {};
        inp.type = INPUT_MOUSE;
        inp.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
        SendInput(1, &inp, sizeof(INPUT));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        inp.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
        SendInput(1, &inp, sizeof(INPUT));
    }
    void moveTo(int x, int y) { SetCursorPos(x, y); }
private:
    InterceptionDriver* m_driver;
    Humanizer* m_humanizer;
};

} // namespace Livn

#pragma once
#include <string>
#include <iostream>
#include <mutex>

namespace Livn {

class Logger {
public:
    void info(const std::string& msg) {
        std::lock_guard<std::mutex> lock(m_mtx);
        std::cout << "[INFO] " << msg << std::endl;
    }
    void warn(const std::string& msg) {
        std::lock_guard<std::mutex> lock(m_mtx);
        std::cout << "[WARN] " << msg << std::endl;
    }
    void error(const std::string& msg) {
        std::lock_guard<std::mutex> lock(m_mtx);
        std::cerr << "[HATA] " << msg << std::endl;
    }
private:
    std::mutex m_mtx;
};

} // namespace Livn

cmake_minimum_required(VERSION 3.20)
project(LivnMacro VERSION 4.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ── Bağımlılıklar ──
# OpenCV: ekran yakalama, template matching, pixel analiz
# Tesseract: OCR (mob ismi, chat okuma)
# nlohmann/json: config dosyası
# Interception: HW level input (lib/ klasöründe)

find_package(OpenCV REQUIRED)

# Tesseract: vcpkg lib path'ini doğrudan belirt
find_package(Tesseract CONFIG REQUIRED)
link_directories(${_VCPKG_INSTALLED_DIR}/x64-windows/lib)

# Interception driver lib (lib/ altına kopyalanacak)
set(INTERCEPTION_LIB ${CMAKE_SOURCE_DIR}/lib/interception.lib)
set(INTERCEPTION_DLL ${CMAKE_SOURCE_DIR}/lib/interception.dll)

# nlohmann/json vcpkg tarafından sağlanıyor, ayrı include gerekmez
find_package(nlohmann_json CONFIG REQUIRED)

# ── Kaynak dosyalar ──
set(CORE_SRC
    src/core/app.cpp
    src/core/config.cpp
    src/core/calibration.cpp
    src/core/hotkey.cpp
    src/core/logger.cpp
)

set(VISION_SRC
    src/vision/screen_capture.cpp
    src/vision/pixel_analyzer.cpp
    src/vision/ocr_engine.cpp
    src/vision/roi_manager.cpp
    src/vision/icon_tracker.cpp
)

set(INPUT_SRC
    src/input/interception_driver.cpp
    src/input/key_sender.cpp
    src/input/mouse_sender.cpp
    src/input/humanizer.cpp
)

set(MACRO_SRC
    src/macro/macro_engine.cpp
    src/macro/combo_runner.cpp
    src/macro/cd_tracker.cpp
    src/macro/mage_module.cpp
    src/macro/priest_module.cpp
    src/macro/rogue_module.cpp
    src/macro/warrior_module.cpp
    src/macro/extra_module.cpp
)

set(UI_SRC
    src/ui/overlay.cpp
    src/ui/hud.cpp
)

# ── Ana executable ──
add_executable(LivnMacro
    src/main.cpp
    ${CORE_SRC}
    ${VISION_SRC}
    ${INPUT_SRC}
    ${MACRO_SRC}
    ${UI_SRC}
)

target_include_directories(LivnMacro PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${OpenCV_INCLUDE_DIRS}
    ${Tesseract_INCLUDE_DIRS}
)

target_link_libraries(LivnMacro PRIVATE
    ${OpenCV_LIBS}
    Tesseract::libtesseract
    nlohmann_json::nlohmann_json
    ${CMAKE_SOURCE_DIR}/lib/interception.lib
    dwmapi
    d3d11
    dxgi
)

# ── Config dosyası kopyala ──
configure_file(${CMAKE_SOURCE_DIR}/config/default.json ${CMAKE_BINARY_DIR}/bin/config.json COPYONLY)

# ── Kalibrasyon aracı ──
add_executable(Calibrator
    src/tools/calibrator.cpp
)

target_include_directories(Calibrator PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(Calibrator PRIVATE
    ${OpenCV_LIBS}
    nlohmann_json::nlohmann_json
)

# ── Post-build: DLL kopyala ──
add_custom_command(TARGET LivnMacro POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${INTERCEPTION_DLL} $<TARGET_FILE_DIR:LivnMacro>
)

# Livn Macro v4.1 — Derleme Talimatları

## Gereksinimler

| Bileşen | Versiyon | İndirme |
|---------|----------|---------|
| Visual Studio 2022 | Community+ | https://visualstudio.microsoft.com/ |
| CMake | 3.20+ | https://cmake.org/download/ |
| OpenCV | 4.8+ | https://opencv.org/releases/ |
| Tesseract OCR | 5.x | https://github.com/UB-Mannheim/tesseract/wiki |
| nlohmann/json | 3.x | https://github.com/nlohmann/json (header-only) |
| Interception | 1.0 | https://github.com/oblitum/Interception |

## Kurulum Adımları

### 1. Bağımlılıkları Kur

```powershell
# vcpkg ile (önerilen)
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install opencv4:x64-windows
.\vcpkg install tesseract:x64-windows
.\vcpkg install nlohmann-json:x64-windows
```

### 2. Interception Driver

```powershell
# GitHub'dan indir
git clone https://github.com/oblitum/Interception.git

# Driver kur (yönetici olarak)
cd Interception\command line installer
install-interception.exe /install

# Bilgisayarı yeniden başlat (driver yüklenmesi için şart)

# lib ve dll dosyalarını projeye kopyala
copy Interception\library\x64\interception.lib  livn-macro\lib\
copy Interception\library\x64\interception.dll  livn-macro\lib\
```

### 3. Tesseract Veri Dosyası

```powershell
# tessdata klasörünü oluştur
mkdir livn-macro\bin\tessdata

# İngilizce dil dosyasını indir
curl -L -o livn-macro\bin\tessdata\eng.traineddata ^
  https://github.com/tesseract-ocr/tessdata/raw/main/eng.traineddata
```

### 4. Derleme

```powershell
cd livn-macro
mkdir build
cd build

# vcpkg ile
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake

# Derle
cmake --build . --config Release

# Çalıştırılabilir: build\bin\Release\LivnMacro.exe
```

### 5. Çalıştırma

```powershell
cd build\bin\Release

# Config dosyasını kontrol et
# config.json — tüm ayarlar burada

# Yönetici olarak çalıştır (Interception driver için şart)
LivnMacro.exe
```

## İlk Kullanım

1. Knight Online'ı başlat
2. LivnMacro.exe'yi yönetici olarak çalıştır
3. Kalibrasyon: program ekranda ROI seçimi isteyecek
4. config.json'dan skill slot'larını ayarla
5. Shortcut tuşlarına bas → macro çalışmaya başlar

## Proje Yapısı

```
livn-macro/
├── CMakeLists.txt          # Build sistemi
├── config/
│   └── default.json        # Varsayılan config (skill slot, ROI, shortcut)
├── src/
│   ├── main.cpp            # Giriş noktası
│   ├── core/
│   │   ├── app.h/cpp       # Ana uygulama, thread yönetimi
│   │   ├── config.h/cpp    # JSON config yükleme/kaydetme
│   │   ├── calibration.*   # ROI kalibrasyon sistemi
│   │   ├── hotkey.*        # Hold/Toggle shortcut dinleme
│   │   └── logger.*        # Loglama
│   ├── vision/
│   │   ├── screen_capture.*    # BitBlt ekran yakalama
│   │   ├── pixel_analyzer.*    # HP/MP bar, debuff renk analizi
│   │   ├── ocr_engine.*        # Tesseract OCR (mob ismi, chat)
│   │   ├── roi_manager.*       # ROI bölge yönetimi
│   │   └── icon_tracker.*      # Buff/CD ikon parlaklık takibi
│   ├── input/
│   │   ├── interception_driver.*  # HW level input
│   │   ├── key_sender.*       # Tuş gönderme (F tab + slot)
│   │   ├── mouse_sender.*     # Fare tıklama
│   │   └── humanizer.*        # İnsan benzeri gecikme
│   ├── macro/
│   │   └── macro_engine.*     # Tüm sınıf modülleri (karar motoru)
│   └── ui/
│       ├── overlay.*          # Topmost transparent pencere
│       └── hud.*              # HUD bilgi gösterimi
├── lib/
│   ├── interception.lib       # Interception driver lib
│   └── interception.dll       # Interception driver dll
├── include/
│   └── nlohmann/json.hpp      # Header-only JSON kütüphanesi
└── docs/
    └── BUILD.md               # Bu dosya
```

## Thread Mimarisi

```
┌─────────────────────────────────────────────┐
│                  main()                      │
│                    │                         │
│     ┌──────────────┼──────────────┐          │
│     │              │              │          │
│  Vision         Macro          Hotkey        │
│  Thread         Thread         Thread        │
│  (~60fps)       (~1ms)        (~1ms)         │
│     │              │              │          │
│  ScreenCap    MacroEngine    HotkeyMgr       │
│  → ROI cut    → tick()       → poll()        │
│  → Pixel      → Karar ver   → Hold/Toggle   │
│  → OCR        → Input gön.  → Combo aç/kapa │
│     │              │                         │
│     └──────── GameState ──────┘              │
│           (mutex korumalı)                   │
│                                              │
│              HUD Thread                      │
│              → Overlay güncelle              │
└─────────────────────────────────────────────┘
```

## Config Düzenleme

config.json'da her skill için F tab ve slot ayarlanır:

```json
"attack_skill": { "f_tab": "F1", "slot": "2" }
```

Bu: F1 tab'ındaki 2 numaralı slot demek.
Macro çalışırken önce F1'e basar (tab değiştirir), sonra 2'ye basar (skill'i tetikler).

Hold/Toggle modu:
```json
"shortcut": "CapsLock",
"mode": "hold"
```
- **hold**: CapsLock basılı = çalışır, bırakınca durur
- **toggle**: CapsLock bir bas = başla, tekrar bas = dur

## Sorun Giderme

- **"Interception driver yüklenemedi"**: Yönetici olarak çalıştırın + bilgisayarı yeniden başlatın
- **"Attack Failed" çok sık**: config.json'da delay_ms değerini artırın (300 → 350)
- **OCR çalışmıyor**: tessdata/eng.traineddata dosyasının varlığını kontrol edin
- **Ekran yakalama boş**: Hedef EXE adını kontrol edin, oyun tam ekran yerine pencere modunda deneyin

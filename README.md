# NeuroFly 🚀

**NeuroFly**, K-12 düzeyinde (Hisar IdeaLab / FabLab / MIT Edgerton Center / Fab Academy ruhuyla) geliştirilen açık kaynaklı, modüler, iç mekan (indoor) mikro dron ve akıllı kumanda platformudur.

Proje; dijital üretim (lazer kesim & 3D baskı), gömülü sistemler (Seeed Studio XIAO ESP32-C6), kablosuz telemetri (ESP-NOW) ve yapay zeka/otonom uçuş yarışmalarını tek bir ekosistemde buluşturur.

---

## 🎯 Temel Hedefler & Vizyon

1. **Dijital Üretim (Digital Fabrication):**
   - Şase: Lazer kesim (pleksi / kavak kontraplak / delrin).
   - Koruma & Aksesuarlar: 3D baskı (PLA / hafif ve esnek TPU pervane kanalları / ducts).
2. **Güvenli Kapalı Alan Mimarisi:**
   - 1S LiPo batarya ile çalışan hafif coreless (fırçalı) motorlar.
   - Pervane koruma kafesleri sayesinde sınıf/atölye ortamında güvenli uçuş.
3. **Akıllı Çift Yönlü Kumanda:**
   - Seeed Studio XIAO ESP32-C6 tabanlı.
   - OLED/TFT durum ekranı, analog/dijital yön butonları veya mini joystickler.
   - Düşük gecikmeli ESP-NOW telemetri ve komut kanalı.
4. **Çok Katmanlı Yarışma (Competition Pillars):**
   - **Manual Pilot:** Kumanda ile hassas kapalı alan parkur ve slalom görevi.
   - **Autonomous Navigation:** Sensör tabanlı (Time-of-Flight / mesafe) otonom engel sakınma veya sabit irtifa tutma.
   - **AI Challenges:** Kamera/görüntü işleme veya harici AI modelleriyle entegre hedef tespit ve karar mekanizmaları.

---

## 📁 Proje Yapısı

```text
neurofly/
├── firmware/
│   ├── drone/          # Drone uçuş kontrolörü yazılımı (PID, IMU, Motor PWM, Failsafe)
│   ├── controller/     # Kumanda yazılımı (Giriş okuma, OLED UI, ESP-NOW yayını)
│   └── common/         # Ortak ESP-NOW veri paketleri ve sabitler (protocol.h)
├── hardware/
│   ├── cad/            # 3D baskı dosyaları (STL, STEP pervane korumaları, kumanda kasası)
│   ├── laser/          # Lazer kesim şase şablonları (DXF, SVG)
│   └── schematics/     # Bağlantı şemaları, pinout ve devre tasarımları
└── docs/
    ├── assembly/       # Adım adım montaj rehberleri ve öğrenci yönergeleri
    ├── bom/            # Malzeme listesi (BOM) ve tedarik kaynakları
    ├── competition/    # Yarışma formatları, parkur tasarımı ve puanlama matrisi
    └── ai_autonomous/  # Otonom uçuş ve AI görev senaryoları
```

---

## 🛠️ Donanım Özeti

- **Ana İşlemci:** [Seeed Studio XIAO ESP32-C6](https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/) (RISC-V, Wi-Fi 6, BLE 5, 802.15.4 / ESP-NOW desteği)
- **Drone Sensör:** 6-eksen IMU (MPU6050 veya LSM6DS3) + Opsiyonel VL53L0X ToF Mesafe Sensörü
- **Motorlar:** 4x Coreless DC motor (örn. 716 veya 8520) + MOSFET sürücüler (SI2302 / AO3400)
- **Kumanda:** XIAO ESP32-C6 + 0.96" I2C OLED (SSD1306) + Yön butonları/mini joystickler
- **Güç:** 1S 3.7V 300-500mAh LiPo batarya

---

## 📜 Lisans & Açık Kaynak
Bu proje eğitim ve FabLab topluluğu için açık kaynak olarak geliştirilmektedir.

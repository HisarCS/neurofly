# NeuroFly - Malzeme Listesi (BOM)

Bu liste, hem **NeuroFly Drone** hem de **NeuroFly Kumanda** için gerekli temel bileşenleri ve FabLab dijital üretim malzemelerini içerir.

---

## 1. NeuroFly Drone (Uçuş Modülü)

| Parça | Adet | Açıklama / Model Örneği | Notlar / FabLab Üretimi |
| :--- | :---: | :--- | :--- |
| **Geliştirme Kartı** | 1 | Seeed Studio XIAO ESP32-C6 | Dahili anten veya harici anten konnektörü |
| **IMU Sensörü** | 1 | MPU6050 veya LSM6DS3 | 6 Eksen Jiroskop + İvmeölçer (I2C) |
| **Opsiyonel Sensör** | 1 | VL53L0X veya VL53L1X | ToF Lazer Mesafe Sensörü (İrtifa sabitleme / engel algılama) |
| **Motorlar** | 4 | 716 (7x16mm) veya 8520 Coreless DC | 2x CW (Saat yönü), 2x CCW (Ters saat yönü) |
| **Pervaneler** | 4 | 31mm - 55mm Mikro Pervane | 2x CW, 2x CCW |
| **Motor Sürücüler** | 4 | N-Kanal Logic-Level MOSFET (AO3400 / SI2302) | 3.3V gate gerilimi ile tam iletime geçen MOSFET |
| **Koruma Diyotları**| 4 | Schottky Diyot (1N5819 veya SS14) | Motor geri EMK akımını sönümlemek için paralel |
| **Dirençler** | 4 | 10kΩ & 4x 100Ω | Gate pull-down ve gate seri akım sınırlayıcı |
| **Batarya** | 1 | 1S 3.7V 300mAh - 450mAh LiPo (25C-30C) | JST-PH 2.0 veya BT2.0 konnektörlü |
| **Şase (Ana Gövde)** | 1 | 2mm veya 3mm Pleksi / Kontraplak | **Lazer Kesim** (FabLab) |
| **Pervane Koruması** | 1-4 | TPU veya PLA Hafif Kanallar (Ducts) | **3D Baskı** (FabLab) |

---

## 2. NeuroFly Kumanda (Transmitter Modülü)

| Parça | Adet | Açıklama / Model Örneği | Notlar / FabLab Üretimi |
| :--- | :---: | :--- | :--- |
| **Geliştirme Kartı** | 1 | Seeed Studio XIAO ESP32-C6 | Drone ile ESP-NOW üzerinden haberleşir |
| **Ekran** | 1 | 0.96" I2C OLED (SSD1306, 128x64) | Batarya seviyesi, modlar ve sinyal gücü göstergesi |
| **Giriş Elemanları** | 4-6 | 12x12mm Push Buton veya 2x Mini Analog Joystick | Throttle, Yaw, Pitch, Roll kontrolleri için |
| **Batarya / Güç** | 1 | 1S LiPo (500-1000mAh) + TP4056 Şarj Modülü | veya USB-C Power Bank doğrudan bağlantısı |
| **Kumanda Kasası** | 1 | Ergonomik kumanda kabuğu | **3D Baskı** (PLA/PETG) veya **Lazer Katmanlı Kasa** |

---

## 3. Üretim ve Montaj Sarf Malzemeleri

- İnce çok telli silikon kablo (28-30 AWG)
- Lehim teli ve havya
- Çift taraflı sünger bant (IMU titreşim izolasyonu için)
- M2 vida ve somun / standoff (naylon tercih edilir)
- Isıyla daralan makaron (shrink tube)

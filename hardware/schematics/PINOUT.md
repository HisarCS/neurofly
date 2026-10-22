# NeuroFly Donanım & Pinout Spesifikasyonu

Bu doküman, Seeed Studio XIAO ESP32-C6 geliştirme kartının **Drone** ve **Kumanda** modüllerindeki pin eşleşmelerini ve KiCad tasarım kriterlerini tanımlar.

---

## 1. Seeed Studio XIAO ESP32-C6 Pin Özeti

| Pin Adı | ESP32-C6 GPIO | Fonksiyonlar |
| :--- | :--- | :--- |
| **D0** | GPIO0 | ADC1_CH0, LP_GPIO0 |
| **D1** | GPIO1 | ADC1_CH1, LP_GPIO1 |
| **D2** | GPIO2 | ADC1_CH2 |
| **D3** | GPIO21 | PWM, General IO |
| **D4** | GPIO22 | I2C SDA (Dahili I2C Varsayılan) |
| **D5** | GPIO23 | I2C SCL (Dahili I2C Varsayılan) |
| **D6** | GPIO16 | TX (UART) / PWM |
| **D7** | GPIO17 | RX (UART) / PWM |
| **D8** | GPIO18 | PWM, General IO |
| **D9** | GPIO19 | PWM, General IO |
| **D10** | GPIO20 | PWM, General IO |

---

## 2. NeuroFly Drone (Uçuş Kontrol Kartı) Pin Dağılımı

Drone gövdesinde XIAO C6 bir taşıyıcı PCB (Carrier Board) üzerine takılır.

| Bağlantı | XIAO Pin | ESP32-C6 GPIO | Açıklama |
| :--- | :--- | :--- | :--- |
| **Motor 1 (Ön Sol - CW)** | D0 | GPIO0 | PWM Çıkışı -> N-FET Gate (AO3400) |
| **Motor 2 (Ön Sağ - CCW)** | D1 | GPIO1 | PWM Çıkışı -> N-FET Gate (AO3400) |
| **Motor 3 (Arka Sağ - CW)** | D2 | GPIO2 | PWM Çıkışı -> N-FET Gate (AO3400) |
| **Motor 4 (Arka Sol - CCW)** | D3 | GPIO21 | PWM Çıkışı -> N-FET Gate (AO3400) |
| **I2C SDA** | D4 | GPIO22 | MPU6050 (IMU) & VL53L0X (ToF) SDA |
| **I2C SCL** | D5 | GPIO23 | MPU6050 (IMU) & VL53L0X (ToF) SCL |
| **Batarya Voltaj Takibi** | D6 | GPIO16 | 1S LiPo Voltaj Bölücü (2x 10kΩ) -> ADC Girişi |
| **Status LED / Beeper** | D7 | GPIO17 | Durum LED'i / Mini Piezo Buzzer |
| **ToF XSHUT (Opsiyonel)** | D8 | GPIO18 | ToF Mesafe Sensörü Uyandırma/Kapatma |

### Motor Sürücü Devresi (Her Motor İçin):
```text
                  +3.7V (1S LiPo)
                       │
                     ┌─┴─────────┐
                     │           │
                 [Motor]      [1N5819 Schottky]
                     │        (Katot yukarıda)
                     │           │
                     └─┬─────────┘
                       │
                    Drain (D)
GPIO PWM ──[100Ω]───Gate (G) [AO3400 N-MOSFET]
                       │
                  Source (S)
                       │
                      GND
                       │
       (Gate-GND arasına 10kΩ Pull-down direnci)
```

---

## 3. NeuroFly Kumanda (Transmitter) Pin Dağılımı

Kumanda hem bağımsız bir RF verici (ESP-NOW) hem de USB-C üzerinden Web Simülatör kontrolcüsü (Web Serial / Gamepad) olarak çalışır.

| Giriş / Çıkış | XIAO Pin | ESP32-C6 GPIO | Açıklama |
| :--- | :--- | :--- | :--- |
| **OLED SDA** | D4 | GPIO22 | 0.96" SSD1306 OLED Ekran SDA |
| **OLED SCL** | D5 | GPIO23 | 0.96" SSD1306 OLED Ekran SCL |
| **Throttle / Gaz Buton/Pot** | D0 | GPIO0 | Yukarı/Aşağı Butonları veya Sol Joystick Y |
| **Yaw / Dönüş Buton/Pot** | D1 | GPIO1 | Sol/Sağ Butonları veya Sol Joystick X |
| **Pitch / Eğilme Buton/Pot**| D2 | GPIO2 | İleri/Geri Butonları veya Sağ Joystick Y |
| **Roll / Yatış Buton/Pot** | D3 | GPIO21 | Sağ/Sol Butonları veya Sağ Joystick X |
| **Arm / Disarm Butonu** | D8 | GPIO18 | Güvenlik motor kilitleme/açma butonu |
| **Mod Seçim Butonu** | D9 | GPIO19 | Manuel / Alt-Hold / Autonomous Geçiş |

---

## 4. KiCad Tasarım Kriterleri (FabLab & Fab Academy Standartları)

1. **İki Yönlü Üretim Desteği:**
   - **CNC Kazıma (FabLab Milling - Roland SRM-20 vb.):** 
     - Tek yüzlü (Single-layer) veya çift yüzlü kolay hizalanabilir tasarım.
     - Minimum hat kalınlığı (Trace width): 0.4mm (16 mil).
     - Minimum yalıtım mesafesi (Clearance): 0.4mm (16 mil).
   - **Fabrika Üretimi (JLCPCB / PCBWay vb.):**
     - SMD 0805 veya 1206 komponent kütüphaneleri (öğrencilerin kolay lehimleyebilmesi için).
2. **Ağırlık Tasarrufu:**
   - Taşıyıcı PCB'de gereksiz FR4 alanları boşaltılarak (cutout) şase ağırlığı minimuma indirilir.

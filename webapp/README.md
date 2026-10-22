# NeuroFly Web Simülatör & Telemetri İstasyonu 🎮

Bu modül, öğrencilerin kendi ürettikleri fiziksel kumandayı (Seeed Studio XIAO ESP32-C6) doğrudan bilgisayara USB-C ile bağlayarak tarayıcı üzerinden uçuş pratiği yapmalarını, PID kontrol algoritmalarını test etmelerini ve yarışma parkuruna hazırlanmalarını sağlar.

---

## 🚀 Özellikler

1. **Sıfır Kurulum (Zero Install):**
   - Herhangi bir harici simülatör veya sürücü kurmaya gerek yoktur.
   - Doğrudan Google Chrome / Microsoft Edge üzerinden Web Serial API ile çalışır.
2. **Üç Boyutlu Parkur (Three.js):**
   - FabLab yarışma kapıları (gates) ve zemin gridi.
   - Pervane korumalı gerçekçi drone modeli.
3. **Telemetri HUD:**
   - İrtifa, gaz (throttle), ARM/DISARM durumu ve tur/puan takibi.

---

## 🏃 Çalıştırma

Basit bir yerel HTTP sunucusu ile hemen başlatılabilir:

```bash
cd webapp/public
python3 -m http.server 8000
```

Ardından tarayıcınızdan `http://localhost:8000` adresine gidin.

- **Kumanda ile:** Kumandayı USB-C ile bilgisayara takın, üst kısımdaki **"Web Serial Kumanda Bağla"** butonuna basarak XIAO ESP32-C6 portunu seçin.
- **Klavye ile Test:** Kumanda bağlı değilse klavyeden <kbd>Space</kbd> ile motorları ARM edip <kbd>W</kbd>/<kbd>S</kbd> ve yön tuşlarıyla uçabilirsiniz.

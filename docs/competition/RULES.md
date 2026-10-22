# NeuroFly Challenge & Yarışma Formatı 🏆

Hisar IdeaLab / FabLab ortamında düzenlenen NeuroFly etkinliğinde kazanan; **üretim verimliliği**, **pilotaj performansı** ve **otonom yazılım zekasının** dengeli bir bileşimiyle belirlenir.

---

## 🏆 Büyük Ödül Puanlama Formülü (Grand Prix Scoring)

Toplam puan aşağıdaki 3 ana sütunun ağırlıklı toplamıyla hesaplanır:

$$\text{Toplam Skor} = P_{\text{Ağırlık}} + P_{\text{Tur}} + P_{\text{Otonom/AI}}$$

### 1. Sütun: Ağırlık Verimliliği ($P_{\text{Ağırlık}}$) - "En Hafif Üretim"
FabLab üretiminde hafiflik = daha uzun uçuş süresi, daha yüksek çeviklik ve daha az batarya tüketimi.
- Tüm dronelar tartılır (batarya dahil Uçuşa Hazır Ağırlık - AUW).
- **Referans Baz Ağırlık:** 45 gram.
- **Puanlama:** Belirlenen baz ağırlıktan daha hafif olan her gram için ekstra puan eklenir. Yapısal rijitliği korurken en hafif şaseyi lazer/3D ile üreten ve en temiz lehimlemeyi/PCB'yi tasarlayan takım zirveye çıkar.

### 2. Sütun: Hız ve Dayanıklılık ($P_{\text{Tur}}$) - "En Fazla Tur Bindiren"
Belirlenen süre (örn. 3 dakika) boyunca FabLab parkurunda yarış:
- Kumanda ile manuel kontrol.
- Her tamamlanan temiz kapı geçişi ve tam tur: **+10 Puan**.
- Kaza/çarpma sonrası yeniden kalkış süresi pilotun süresinden yer.
- Süre dolduğunda en çok tur atan takım maksimum yarış puanını alır.

### 3. Sütun: Otonom & AI Görevleri ($P_{\text{Otonom/AI}}$)
Öğrencilerin yazdığı algoritmalar ve sensör kullanımı:
- **Hassas Kalkış ve İrtifa Kilidi (Alt-Hold):** Pilot dokunmadan 15 sn sabit irtifada kalma: **+50 Puan**.
- **Otonom Engelden Kaçış / Duvar Algılama:** ToF sensörüyle engeli görüp çarpmadan durma/geri çekilme: **+75 Puan**.
- **AI Hedef / ArUco / Renk Tanıma:** Kamera veya harici AI istasyonuyla yönlendirilerek hedefe ulaşma: **+100 Puan**.

---

## 💻 Web App & Uçuş Simülatörü

Fiziksel uçuş öncesinde öğrencilerin hem kodlarını hem de pilotajlarını güvenle geliştirebilmeleri için Web tabanlı simülasyon ortamı sağlanır:
- **Kumanda Entegrasyonu:** Tasarlanan XIAO ESP32-C6 kumanda, USB-C kablosu üzerinden **Web Serial API** (veya Web HID / Web Bluetooth) ile doğrudan tarayıcıya bağlanır.
- **Gerçek Zamanlı Simülasyon:** WebGL/Three.js tabanlı 3D FabLab ortamında kumanda tepkileri birebir test edilir.
- **Sanal Parkur & Challenge Antrenmanı:** Öğrenciler fiziksel droneları kırmadan önce sanal engellerden geçip tur sürelerini simülatörde optimize edebilirler.
- **Algoritma Testi:** Otonom PID katsayıları ve kontrol yazılımları web simülatöründe ön teste tabi tutulabilir.

---

## 🛡️ Güvenlik & Diskalifiye Kuralları

1. **Pervane Koruması Şartı:** 3D baskı koruma kanalları (ducts) olmadan hiçbir drone yarışmaya kabul edilmez.
2. **Yazılımsal Failsafe:** Kumanda bağlantısı kesildiğinde motorlar en geç 200ms içinde tamamen durmalıdır.
3. **Puan Kırma / Ceza:** Parkur dışına kontrolsüz çıkış veya hakem/izleyici güvenliğini tehdit eden uçuşlarda tur iptal edilir.

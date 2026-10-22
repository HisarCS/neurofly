# NeuroFly Challenge & Yarışma Formatı 🏆

Hisar IdeaLab / FabLab ortamında düzenlenecek NeuroFly etkinliği, öğrencilerin mekanik tasarım, elektronik montaj, kodlama, pilotaj ve yapay zeka entegrasyonu becerilerini test eden 3 ana etaptan oluşur.

---

## 1. Kademe: Dijital Üretim ve Montaj (Assembly & Craftsmanship)

- **Şase İnovasyonu:** Lazer kesim şase ve 3D baskı koruyucuların hafiflik, rijitlik ve estetik dengesi.
- **Lehimleme & Temiz Kablolama:** Kısa devre olmaması, ağırlık merkezinin (CoG) batarya ile doğru dengelenmesi.
- **Ağırlık Kısıtı:** Uçuşa hazır toplam ağırlık (AUW - All Up Weight) hedefi: **< 45 - 55 gram** (1S LiPo dahil).

---

## 2. Kademe: Manuel Pilotaj & Slalom (Manual Agility)

- **Hedef:** Öğrencilerin tasarladığı kumanda ile drone'u havalandırıp FabLab içerisindeki belirlenen kapılardan (gate) geçirmek.
- **Puanlama Kriterleri:**
  - Parkuru tamamlama süresi.
  - Engellere çarpmama (koruma duct'ları sayesinde güvenli temas serbest, düşme ceza puanı).
  - Belirlenen hedef iniş platformuna (Landing Pad) hassas iniş.

---

## 3. Kademe: Otonom Görev (Autonomous Navigation)

- **Hedef:** Drone'un üzerindeki sensörler (örn. ToF lazer mesafe sensörü veya optik sensör) yardımıyla pilot müdahalesi olmadan belirli manevraları gerçekleştirmesi.
- **Görevler:**
  - **Otonom Kalkış & İrtifa Sabitleme (Alt Hold):** Yerden 50 cm yükselip 15 saniye boyunca irtifasını koruma.
  - **Duvar / Engel Algılama:** Karşısına çıkan engeli ToF ile fark edip geri çekilme veya durma.

---

## 4. Kademe: Gelecek Vizyonu - AI Challenge (Future Track)

- **Görüntü İşleme / Harici AI Entegrasyonu:**
  - FabLab tavanına veya kenarına yerleştirilmiş harici bir kamera (Edge AI / OpenCV) aracılığıyla drone üzerindeki renkli/ArUco işaretçinin takip edilmesi.
  - Veya Wi-Fi / ESP-NOW üzerinden yer istasyonuna (PC/Raspberry Pi) aktarılan telemetri verilerine göre rota optimizasyonu sağlayan yapay zeka kontrolü.
  - Dronun otonom olarak en kestirme rotayı hesaplayıp hedefe ulaşması.

---

## 🛡️ Güvenlik Kuralları (Safety First)

1. **Kapalı Pervane Koruması:** Pervanelerin açıkta kalmasına izin verilmez; 3D baskı koruma halkaları (ducts/guards) zorunludur.
2. **Failsafe:** Kumanda sinyali koptuğunda veya acil durum butonuna basıldığında motorlar 200 ms içinde anında durmalıdır (`DISARM`).
3. **Göz Koruması:** Uçuş testleri ve yarışma sırasında pilotlar ve jüri koruyucu güvenlik gözlüğü takmalıdır.

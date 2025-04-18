# Arduino Konfigürasyon ve Kontrol Paneli

Bu proje, bir Arduino cihazı ile haberleşerek motor pinlerini konfigüre etmenizi ve cihazı web arayüzü üzerinden kontrol etmenizi sağlayan bir Flask uygulamasıdır. Seri port üzerinden Arduino’ya komutlar gönderilir ve yapılandırmalar bir JSON dosyasına kaydedilir. Arduino tarafındaki yazılım, EEPROM kullanarak pin atamalarını ve mod ayarlarını kalıcı olarak saklar ve gelen seri komutlara göre motorları yönlendirir.

---

## 🚀 Özellikler

- 🔌 **Dinamik Port Listesi:** Arduino dahil tüm bağlı seri portlar listelenir ve açıklamalarıyla birlikte gösterilir.
- 🛠️ **Konfigürasyon Yapılandırması:** Web arayüzü üzerinden motor pinleri ve mod ayarları tanımlanabilir.
- 📋 **Veri Listeleme:** Yapılan tüm konfigürasyonlar görüntülenebilir, güncellenebilir ve silinebilir.
- 🎮 **Motor Kontrol Paneli:** Düğmeler veya klavye yön tuşları ile ileri, geri, sağ, sol yönlerinde motor kontrolü yapılabilir.
- 📡 **Arduino’ya Seri Veri Gönderme:** `"P,<pinTipi>,<pinDegeri>"` ve `"I", "G", "L", "R", "S"` gibi sade komut formatları desteklenir.
- 💾 **EEPROM Desteği:** Pin atamaları ve çalışma modları EEPROM üzerinden kaydedilir ve cihaz yeniden başlatıldığında korunur.

---

## 📁 Proje Yapısı

```
proje/
├── app.py                    # Flask ana uygulama
├── arduino_config.json       # Kayıtlı konfigürasyon verileri
├── templates/
│   ├── anasayfa.html         # Ana kontrol ekranı
│   ├── konfigrasyon.html     # Konfigürasyon formu
│   ├── kayitli-veriler.html  # Veri listeleme/güncelleme/silme
│   └── kontrol.html          # Gerçek zamanlı motor kontrol ekranı
├── static/
│   └── styles.css            # (İsteğe bağlı) özel stiller
├── Arduino/                 # Arduino kodları (örnek)
│   └── kontrol_kodu.ino      # EEPROM, motor ve komut kontrolü
├── LICENSE                   # MIT Lisansı
└── README.md                 # Proje açıklamaları
```

---

## 🛠️ Gereksinimler

- Python 3.8 veya üzeri
- Flask
- pyserial
- Arduino IDE (Arduino tarafı için)

### Kurulum

```bash
pip install flask pyserial
```

---

## ⚙️ Kullanım

### Sunucuyu Başlat

```bash
python app.py
```

### Web Sayfaları

| Adres                 | Görev                                     |
|-----------------------|--------------------------------------------|
| `/`                   | Ana sayfa                                 |
| `/konfigrasyon`       | Arduino pin ve mod ayarlarını yapılandır  |
| `/kayitli_veriler`    | Yapılan konfigürasyonları listele/güncelle/sil |
| `/kontrol`            | Gerçek zamanlı motor kontrol paneli       |
| `/get_ports`          | (API) Bağlı port listesini JSON olarak döndürür |

---

## 🔁 Arduino Haberleşme Formatı

### Konfigürasyon Komutu

```text
P,<pinTipi>,<pinDegeri>
```
**Örnek:**  
`P,1,5` → PinTipi 1 (Sol ön motor pin 1), değeri 5

### Kontrol Komutları

| Karakter | Anlamı      |
|----------|-------------|
| `I`      | İleri git    |
| `G`      | Geri git     |
| `L`      | Sola dön     |
| `R`      | Sağa dön     |
| `S`      | Dur          |

Arduino kodu gelen komutları tanır ve EEPROM'dan okunan pin yapılandırmalarıyla motorları kontrol eder. İleri, geri, sağa ve sola hareketler `analogWrite` ve `digitalWrite` kombinasyonlarıyla sağlanır. Hızlanma ve yavaşlama süresi yazılımda parametre olarak ayarlanabilir.

---

## 🎮 Klavye ile Joypad Desteği

- `↑` : İleri (`I`)
- `↓` : Geri (`G`)
- `←` : Sol (`L`)
- `→` : Sağ (`R`)
- `Boşluk (space)` : Dur (`S`)

---

## 📂 Veri Kaydı

Tüm yapılandırmalar `arduino_config.json` dosyasına kaydedilir. JSON dosyası aşağıdaki gibi yapılandırılır:

```json
[
  {
    "ekip_id": "Takım 1",
    "port": "COM3",
    "sol_on1": "D5",
    "sag_arka2": "D6",
    "mod_secimi": "1"
  }
]
```

---

## 💡 Geliştirme Önerileri

- 🔄 Port tarama yenileme butonu
- 📊 Anlık durum gösterimi (bağlı cihaz vs.)
- 🔐 Kullanıcı girişi ile yetki kontrolü
- 🌐 Arduino’dan veri okuma (sensör verisi)
- 💬 Arduino’dan gelen veri ile ekranda durum bildirimi

---

## 👨‍💻 Katkı Sağlamak

Katkıda bulunmak istiyorsanız:

1. Fork’layın
2. Yeni bir branch oluşturun: `feature/yeni-ozellik`
3. Değişikliklerinizi commit edin
4. Branch’inizi push edin
5. Pull Request (PR) gönderin

---

## 📄 Lisans

Bu proje MIT Lisansı ile sunulmuştur. Detaylar için `LICENSE` dosyasını inceleyebilirsiniz.


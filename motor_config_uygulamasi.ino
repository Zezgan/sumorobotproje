#include <EEPROM.h>

// Varsayılan Pin Atamaları
int SOL_ON_MOTOR_PIN = 7;     // L298N IN1
int SOL_ON_MOTOR_PIN2 = 8;    // L298N IN2
int SAG_ARKA_MOTOR_PIN = 12;  // L298N IN3
int SAG_ARKA_MOTOR_PIN2 = 13; // L298N IN4

// Enable Pinleri (PWM için)
int SOL_ENABLE_PIN = 5;  // PWM pini
int SAG_ENABLE_PIN = 6;  // PWM pini

// LED Pinleri
int ON_LED_PIN = 2;
int ARKA_LED_PIN = 3;

// EEPROM Adresleri
#define EEPROM_HIZ_ADRES 0
#define EEPROM_MOD_ADRES 1
#define EEPROM_SOL_ON_PIN1_ADRES 2
#define EEPROM_SOL_ON_PIN2_ADRES 3
#define EEPROM_SAG_ARKA_PIN1_ADRES 4
#define EEPROM_SAG_ARKA_PIN2_ADRES 5
#define EEPROM_SOL_ENABLE_PIN_ADRES 6
#define EEPROM_SAG_ENABLE_PIN_ADRES 7
#define EEPROM_ON_LED_PIN_ADRES 8
#define EEPROM_ARKA_LED_PIN_ADRES 9
#define EEPROM_BEKLEME_SURESI_ADRES 10  // 2 byte kullanacak
#define EEPROM_HIZLANMA_SURESI_ADRES 12 // Hızlanma süresi için

// Mod Sabitleri
#define MOD_OTOMATIK 0
#define MOD_KLAVYE 1
#define MOD_BLUETOOTH 2

// Değişkenler
int hiz = 200;          // 0-255 arası
int mod = MOD_OTOMATIK; // Başlangıç modu
unsigned long sonZaman = 0;
bool ileriMi = true;
int beklemeSuresi = 5000; // 5 saniye
int hizlanmaSuresi = 500; // Motorların 0'dan tam hıza çıkma süresi (ms)
int sonDurum = -1;  // Son hareket durumu (-1=başlangıç, 0=durma, 1=ileri, 2=geri, 3=sol, 4=sağ)

void setup() {
  // Seri haberleşmeyi başlat
  Serial.begin(9600);
  Serial.println("Araba Kontrol Sistemi Başlatıldı");
  
  // PWM frekansını ayarla (pin 5 ve 6 için)
  // Bu ayar, motorların düşük hızlarda daha dengeli çalışmasını sağlar
  // Timer0 - Pin 5 ve 6 için (varsayılan: 976 Hz)
  TCCR0B = TCCR0B & B11111000 | B00000010; // 3921 Hz olarak ayarlar
  
  // EEPROM'dan kaydedilmiş ayarları oku
  eepromunuOku();
  
  // Pin modlarını ayarla
  pinleriAyarla();
  
  komutlariGoster();

  // Başlangıçta motorları durdur
  motorlariDurdur();
}

void eepromunuOku() {
  hiz = EEPROM.read(EEPROM_HIZ_ADRES);
  if (hiz == 0 || hiz > 255) {
    hiz = 200; // Geçersiz değer ise varsayılan hız
  }
  
  mod = EEPROM.read(EEPROM_MOD_ADRES);
  if (mod > 2) {
    mod = MOD_OTOMATIK; // Geçersiz değer ise varsayılan mod
  }
  
  // Pin atamaları
  int tempPin = EEPROM.read(EEPROM_SOL_ON_PIN1_ADRES);
  if (tempPin > 0 && tempPin < 14) SOL_ON_MOTOR_PIN = tempPin;
  
  tempPin = EEPROM.read(EEPROM_SOL_ON_PIN2_ADRES);
  if (tempPin > 0 && tempPin < 14) SOL_ON_MOTOR_PIN2 = tempPin;
  
  tempPin = EEPROM.read(EEPROM_SAG_ARKA_PIN1_ADRES);
  if (tempPin > 0 && tempPin < 14) SAG_ARKA_MOTOR_PIN = tempPin;
  
  tempPin = EEPROM.read(EEPROM_SAG_ARKA_PIN2_ADRES);
  if (tempPin > 0 && tempPin < 14) SAG_ARKA_MOTOR_PIN2 = tempPin;
  
  tempPin = EEPROM.read(EEPROM_SOL_ENABLE_PIN_ADRES);
  if (tempPin > 0 && tempPin < 14) SOL_ENABLE_PIN = tempPin;
  
  tempPin = EEPROM.read(EEPROM_SAG_ENABLE_PIN_ADRES);
  if (tempPin > 0 && tempPin < 14) SAG_ENABLE_PIN = tempPin;
  
  tempPin = EEPROM.read(EEPROM_ON_LED_PIN_ADRES);
  if (tempPin > 0 && tempPin < 14) ON_LED_PIN = tempPin;
  
  tempPin = EEPROM.read(EEPROM_ARKA_LED_PIN_ADRES);
  if (tempPin > 0 && tempPin < 14) ARKA_LED_PIN = tempPin;
  
  // Bekleme süresi (2 byte)
  int tempBeklemeSuresi = (EEPROM.read(EEPROM_BEKLEME_SURESI_ADRES) << 8) | EEPROM.read(EEPROM_BEKLEME_SURESI_ADRES + 1);
  if (tempBeklemeSuresi >= 1000 && tempBeklemeSuresi <= 10000) beklemeSuresi = tempBeklemeSuresi;
  
  // Hızlanma süresi
  int tempHizlanmaSuresi = EEPROM.read(EEPROM_HIZLANMA_SURESI_ADRES);
  if (tempHizlanmaSuresi >= 0 && tempHizlanmaSuresi <= 250) {
    hizlanmaSuresi = tempHizlanmaSuresi * 10; // 0-2500ms arasında ayarlanabilir
  }
}

void pinleriAyarla() {
  // Önceden tanımlanmış tüm pinleri sıfırla
  for (int i = 2; i <= 13; i++) {
    pinMode(i, INPUT);
  }
  
  // Motor kontrol pinlerini çıkış olarak ayarla
  pinMode(SOL_ON_MOTOR_PIN, OUTPUT);
  pinMode(SOL_ON_MOTOR_PIN2, OUTPUT);
  pinMode(SAG_ARKA_MOTOR_PIN, OUTPUT);
  pinMode(SAG_ARKA_MOTOR_PIN2, OUTPUT);
  
  // Enable pinlerini çıkış olarak ayarla
  pinMode(SOL_ENABLE_PIN, OUTPUT);
  pinMode(SAG_ENABLE_PIN, OUTPUT);
  
  // LED pinlerini çıkış olarak ayarla
  pinMode(ON_LED_PIN, OUTPUT);
  pinMode(ARKA_LED_PIN, OUTPUT);
  
  // Mevcut pin ayarlarını göster
  Serial.println("Mevcut Pin Ayarları:");
  Serial.print("Sol Ön Motor Pin 1: "); Serial.println(SOL_ON_MOTOR_PIN);
  Serial.print("Sol Ön Motor Pin 2: "); Serial.println(SOL_ON_MOTOR_PIN2);
  Serial.print("Sağ Arka Motor Pin 1: "); Serial.println(SAG_ARKA_MOTOR_PIN);
  Serial.print("Sağ Arka Motor Pin 2: "); Serial.println(SAG_ARKA_MOTOR_PIN2);
  Serial.print("Sol Enable Pin: "); Serial.println(SOL_ENABLE_PIN);
  Serial.print("Sağ Enable Pin: "); Serial.println(SAG_ENABLE_PIN);
  Serial.print("Ön LED Pin: "); Serial.println(ON_LED_PIN);
  Serial.print("Arka LED Pin: "); Serial.println(ARKA_LED_PIN);
  Serial.print("Bekleme Süresi (ms): "); Serial.println(beklemeSuresi);
  Serial.print("Hızlanma Süresi (ms): "); Serial.println(hizlanmaSuresi);
}

void komutlariGoster() {
  Serial.println("\nKullanılabilir Komutlar:");
  Serial.println("H,değer - Hız ayarı (0-255)");
  Serial.println("M,değer - Mod değiştir (0:Otomatik, 1:Klavye, 2:Bluetooth)");
  Serial.println("P,pin_tipi,değer - Pin ayarı");
  Serial.println("  Pin tipleri:");
  Serial.println("  1: Sol Ön Motor Pin 1");
  Serial.println("  2: Sol Ön Motor Pin 2");
  Serial.println("  3: Sağ Arka Motor Pin 1");
  Serial.println("  4: Sağ Arka Motor Pin 2");
  Serial.println("  5: Sol Enable Pin");
  Serial.println("  6: Sağ Enable Pin");
  Serial.println("  7: Ön LED Pin");
  Serial.println("  8: Arka LED Pin");
  Serial.println("B,değer - Bekleme süresi (ms) (1000-10000)");
  Serial.println("A,değer - Hızlanma süresi (0-250, 10ms biriminde)");
  Serial.println("I - İleri git");
  Serial.println("G - Geri git");
  Serial.println("S - Dur");
  Serial.println("L - Sol dön");
  Serial.println("R - Sağ dön");
  Serial.println("? - Komutları göster");
}

void loop() {
  // Seri porttan komut kontrolü
  if (Serial.available() > 0) {
    String komut = Serial.readStringUntil('\n');
    komutIsle(komut);
  }
  
  // Seçilen moda göre işlem yap
  switch (mod) {
    case MOD_OTOMATIK:
      otomatikMod();
      break;
      
    case MOD_KLAVYE:
      // Klavye modu seri port üzerinden komutlarla çalışır
      // Komutlar seri port üzerinden alınacak
      break;
      
    case MOD_BLUETOOTH:
      // Bluetooth modu seri port üzerinden komutlarla çalışır
      // Komutlar seri port üzerinden alınacak
      break;
  }
}

void otomatikMod() {
  unsigned long simdikiZaman = millis();
  
  // Belirlenen sürede bir yön değiştir
  if (simdikiZaman - sonZaman > beklemeSuresi) {
    sonZaman = simdikiZaman;
    ileriMi = !ileriMi;
    
    if (ileriMi) {
      ileriGit();
    } else {
      geriGit();
    }
  }
}

void komutIsle(String komut) {
  komut.trim();
  
  if (komut.length() == 0) return;
  
  char ilkKarakter = komut.charAt(0);
  
  switch (ilkKarakter) {
    case 'H': // Hız ayarı
    case 'h':
      if (komut.indexOf(',') != -1) {
        int yeniHiz = komut.substring(komut.indexOf(',') + 1).toInt();
        if (yeniHiz >= 0 && yeniHiz <= 255) {
          hiz = yeniHiz;
          EEPROM.write(EEPROM_HIZ_ADRES, hiz);
          Serial.print("Hız ayarlandı: ");
          Serial.println(hiz);
        }
      }
      break;
      
    case 'M': // Mod değiştir
    case 'm':
      if (komut.indexOf(',') != -1) {
        int yeniMod = komut.substring(komut.indexOf(',') + 1).toInt();
        if (yeniMod >= 0 && yeniMod <= 2) {
          mod = yeniMod;
          EEPROM.write(EEPROM_MOD_ADRES, mod);
          Serial.print("Mod değiştirildi: ");
          Serial.println(mod);
          
          // Mod değiştiğinde motorları durdur
          motorlariDurdur();
        }
      }
      break;
    
    case 'P': // Pin ayarı
    case 'p':
      if (komutPinAyarla(komut)) {
        // Pin modlarını yeniden ayarla
        pinleriAyarla();
        // Motorları durdur
        motorlariDurdur();
      }
      break;
      
    case 'B': // Bekleme süresi ayarı
    case 'b':
      if (komut.indexOf(',') != -1) {
        int yeniBeklemeSuresi = komut.substring(komut.indexOf(',') + 1).toInt();
        if (yeniBeklemeSuresi >= 1000 && yeniBeklemeSuresi <= 10000) {
          beklemeSuresi = yeniBeklemeSuresi;
          // 2 byte olarak kaydet
          EEPROM.write(EEPROM_BEKLEME_SURESI_ADRES, beklemeSuresi >> 8);
          EEPROM.write(EEPROM_BEKLEME_SURESI_ADRES + 1, beklemeSuresi & 0xFF);
          Serial.print("Bekleme süresi ayarlandı: ");
          Serial.println(beklemeSuresi);
        }
      }
      break;
      
    case 'A': // Hızlanma süresi ayarı
    case 'a':
      if (komut.indexOf(',') != -1) {
        int yeniHizlanmaKatsayi = komut.substring(komut.indexOf(',') + 1).toInt();
        if (yeniHizlanmaKatsayi >= 0 && yeniHizlanmaKatsayi <= 250) {
          hizlanmaSuresi = yeniHizlanmaKatsayi * 10; // 0-2500ms
          EEPROM.write(EEPROM_HIZLANMA_SURESI_ADRES, yeniHizlanmaKatsayi);
          Serial.print("Hızlanma süresi ayarlandı: ");
          Serial.print(hizlanmaSuresi);
          Serial.println("ms");
        }
      }
      break;
      
    case 'I': // İleri git
    case 'i':
      ileriGit();
      break;
      
    case 'G': // Geri git
    case 'g':
      geriGit();
      break;
      
    case 'S': // Dur
    case 's':
      motorlariDurdur();
      break;
      
    case 'L': // Sol dön
    case 'l':
      solaDon();
      break;
      
    case 'R': // Sağ dön
    case 'r':
      sagaDon();
      break;
      
    case '?': // Komutları göster
      komutlariGoster();
      break;
  }
}

bool komutPinAyarla(String komut) {
  // Komut formatı: P,pin_tipi,değer
  int ilkVirgul = komut.indexOf(',');
  if (ilkVirgul == -1) return false;
  
  int ikinciVirgul = komut.indexOf(',', ilkVirgul + 1);
  if (ikinciVirgul == -1) return false;
  
  int pinTipi = komut.substring(ilkVirgul + 1, ikinciVirgul).toInt();
  int pinDeger = komut.substring(ikinciVirgul + 1).toInt();
  
  // Pin değeri geçerlilik kontrolü
  if (pinDeger < 2 || pinDeger > 13) {
    Serial.println("Hata: Pin değeri 2-13 arasında olmalı!");
    return false;
  }
  
  // Pin tipine göre ayarla
  switch (pinTipi) {
    case 1: // Sol Ön Motor Pin 1
      SOL_ON_MOTOR_PIN = pinDeger;
      EEPROM.write(EEPROM_SOL_ON_PIN1_ADRES, pinDeger);
      Serial.print("Sol Ön Motor Pin 1 ayarlandı: ");
      Serial.println(pinDeger);
      break;
      
    case 2: // Sol Ön Motor Pin 2
      SOL_ON_MOTOR_PIN2 = pinDeger;
      EEPROM.write(EEPROM_SOL_ON_PIN2_ADRES, pinDeger);
      Serial.print("Sol Ön Motor Pin 2 ayarlandı: ");
      Serial.println(pinDeger);
      break;
      
    case 3: // Sağ Arka Motor Pin 1
      SAG_ARKA_MOTOR_PIN = pinDeger;
      EEPROM.write(EEPROM_SAG_ARKA_PIN1_ADRES, pinDeger);
      Serial.print("Sağ Arka Motor Pin 1 ayarlandı: ");
      Serial.println(pinDeger);
      break;
      
    case 4: // Sağ Arka Motor Pin 2
      SAG_ARKA_MOTOR_PIN2 = pinDeger;
      EEPROM.write(EEPROM_SAG_ARKA_PIN2_ADRES, pinDeger);
      Serial.print("Sağ Arka Motor Pin 2 ayarlandı: ");
      Serial.println(pinDeger);
      break;
      
    case 5: // Sol Enable Pin
      SOL_ENABLE_PIN = pinDeger;
      EEPROM.write(EEPROM_SOL_ENABLE_PIN_ADRES, pinDeger);
      Serial.print("Sol Enable Pin ayarlandı: ");
      Serial.println(pinDeger);
      break;
      
    case 6: // Sağ Enable Pin
      SAG_ENABLE_PIN = pinDeger;
      EEPROM.write(EEPROM_SAG_ENABLE_PIN_ADRES, pinDeger);
      Serial.print("Sağ Enable Pin ayarlandı: ");
      Serial.println(pinDeger);
      break;
      
    case 7: // Ön LED Pin
      ON_LED_PIN = pinDeger;
      EEPROM.write(EEPROM_ON_LED_PIN_ADRES, pinDeger);
      Serial.print("Ön LED Pin ayarlandı: ");
      Serial.println(pinDeger);
      break;
      
    case 8: // Arka LED Pin
      ARKA_LED_PIN = pinDeger;
      EEPROM.write(EEPROM_ARKA_LED_PIN_ADRES, pinDeger);
      Serial.print("Arka LED Pin ayarlandı: ");
      Serial.println(pinDeger);
      break;
      
    default:
      Serial.println("Hata: Geçersiz pin tipi!");
      return false;
  }
  
  return true;
}

// Yumuşak başlatma ile motor kontrolü (0'dan hedef hıza kademeli geçiş)
void motorlariCalistir(int sol1, int sol2, int sag1, int sag2, int hedefHiz) {
  // Yön pinlerini ayarla
  digitalWrite(SOL_ON_MOTOR_PIN, sol1);
  digitalWrite(SOL_ON_MOTOR_PIN2, sol2);
  digitalWrite(SAG_ARKA_MOTOR_PIN, sag1);
  digitalWrite(SAG_ARKA_MOTOR_PIN2, sag2);
  
  // Eğer hızlanma süresi 0 ise, direkt tam hıza çık
  if (hizlanmaSuresi <= 0) {
    analogWrite(SOL_ENABLE_PIN, hedefHiz);
    analogWrite(SAG_ENABLE_PIN, hedefHiz);
    return;
  }
  
  // Mevcut hızdan hedef hıza kademeli olarak geç
  int baslangicHizi = 0;
  if (sonDurum != 0 && sonDurum != -1) {
    baslangicHizi = hedefHiz / 2; // Yön değişiminde daha yumuşak bir geçiş sağla
  }
  
  unsigned long startTime = millis();
  unsigned long currentTime;
  int currentSpeed;
  
  do {
    currentTime = millis();
    unsigned long elapsedTime = currentTime - startTime;
    
    if (elapsedTime >= hizlanmaSuresi) {
      currentSpeed = hedefHiz;
    } else {
      // Doğrusal interpolasyon: baslangicHizi + (hedefHiz - baslangicHizi) * (elapsedTime / hizlanmaSuresi)
      currentSpeed = baslangicHizi + ((hedefHiz - baslangicHizi) * elapsedTime / hizlanmaSuresi);
    }
    
    analogWrite(SOL_ENABLE_PIN, currentSpeed);
    analogWrite(SAG_ENABLE_PIN, currentSpeed);
    
    // Kısa bir bekleme ekleyerek CPU'yu yormamasını sağla
    delay(5);
    
  } while (currentSpeed < hedefHiz);
}

void ileriGit() {
  // Ön LED'i yak, arka LED'i söndür
  digitalWrite(ON_LED_PIN, HIGH);
  digitalWrite(ARKA_LED_PIN, LOW);
  
  // Motorları ileri yönde çalıştır
  motorlariCalistir(HIGH, LOW, HIGH, LOW, hiz);
  
  Serial.println("İleri gidiliyor");
  sonDurum = 1;
}

void geriGit() {
  // Arka LED'i yak, ön LED'i söndür
  digitalWrite(ON_LED_PIN, LOW);
  digitalWrite(ARKA_LED_PIN, HIGH);
  
  // Motorları geri yönde çalıştır
  motorlariCalistir(LOW, HIGH, LOW, HIGH, hiz);
  
  Serial.println("Geri gidiliyor");
  sonDurum = 2;
}

void motorlariDurdur() {
  // Önce yavaşça durdur (yumuşak duruş)
  if (sonDurum > 0) {
    int yavaslama = hizlanmaSuresi / 2; // Daha kısa sürede yavaşla
    
    int mevcutHiz = hiz;
    for (int i = 0; i < 5; i++) {
      mevcutHiz = mevcutHiz * 0.8; // %20 azalt
      analogWrite(SOL_ENABLE_PIN, mevcutHiz);
      analogWrite(SAG_ENABLE_PIN, mevcutHiz);
      delay(yavaslama / 5);
    }
  }
  
  // Tüm motorları durdur
  digitalWrite(SOL_ON_MOTOR_PIN, LOW);
  digitalWrite(SOL_ON_MOTOR_PIN2, LOW);
  digitalWrite(SAG_ARKA_MOTOR_PIN, LOW);
  digitalWrite(SAG_ARKA_MOTOR_PIN2, LOW);
  
  // Enable pinlerini kapat
  analogWrite(SOL_ENABLE_PIN, 0);
  analogWrite(SAG_ENABLE_PIN, 0);
  
  // Tüm LED'leri söndür
  digitalWrite(ON_LED_PIN, LOW);
  digitalWrite(ARKA_LED_PIN, LOW);
  
  Serial.println("Araç durdu");
  sonDurum = 0;
}

void solaDon() {
  // Sol motorları geri, sağ motorları ileri çalıştır
  motorlariCalistir(LOW, HIGH, HIGH, LOW, hiz);
  
  Serial.println("Sola dönülüyor");
  sonDurum = 3;
}

void sagaDon() {
  // Sol motorları ileri, sağ motorları geri çalıştır
  motorlariCalistir(HIGH, LOW, LOW, HIGH, hiz);
  
  Serial.println("Sağa dönülüyor");
  sonDurum = 4;
}

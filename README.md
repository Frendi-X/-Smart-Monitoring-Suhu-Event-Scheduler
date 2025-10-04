# 🌡️ Smart Monitoring & Event Scheduler

Proyek ini merupakan sistem monitoring suhu dan pengendalian event berbasis Arduino yang dilengkapi dengan sensor DS18B20, RTC, LCD I2C, DFPlayer Mini, dan relay. Alat ini dapat bekerja dalam dua mode, yaitu Auto yang mengikuti waktu nyata dari RTC dan Simulasi yang memungkinkan perhitungan hari secara manual. Sistem mampu memantau suhu, mengendalikan relay secara otomatis sesuai ambang batas suhu, serta menampilkan informasi waktu, suhu, mode, dan status pada LCD. Selain itu, terdapat event otomatis yang terjadwal, yaitu BALIK pada hari ke-3 dan PANEN pada hari ke-6 setelah sistem direset, dengan notifikasi suara dari DFPlayer serta pengiriman data melalui komunikasi serial. Tiga tombol kontrol digunakan untuk mengatur mode, simulasi hari, dan reset sistem, sehingga alat ini dapat berfungsi sebagai pengingat kegiatan berbasis waktu sekaligus pemantau kondisi suhu secara real-time.

---

## ⚙️ Fitur Utama
- **Monitoring Suhu DS18B20**
  - **< 40 °C** → Relay OFF  
  - **> 50 °C** → Relay ON  
  - 40–50 °C → Relay tetap, dengan notifikasi kondisi **NORMAL** terkirim sekali.
- **Mode Operasi**
  - **AUTO** → mengikuti waktu nyata dari RTC DS1307.  
  - **SIMULASI** → perhitungan hari dilakukan secara manual (counter dengan tombol).  
- **Event Terjadwal**
  - **BALIK** → hari ke-3 setelah reset.  
  - **PANEN** → hari ke-6 setelah reset (sekali kirim notifikasi, lalu sistem berhenti).  
- **DFPlayer Mini** untuk audio notifikasi event.  
- **Komunikasi Serial** untuk mengirim data suhu & event ke perangkat lain (ESP32/PC).  
- **LCD 16x2 I2C** untuk menampilkan suhu, waktu, mode, dan status.  
- **Kontrol Tombol**:  
  - `MODE` → memilih Auto/Simulasi.  
  - `SIM` → menambah hari pada mode Simulasi.  
  - `RESET` → memulai ulang sistem dan menandai hari awal.  

---

## 🛠️ Hardware yang Dibutuhkan
- Arduino (UNO / Nano / kompatibel)  
- Sensor suhu **DS18B20**  
- **RTC DS1307**  
- **LCD 16x2 I2C**  
- **DFPlayer Mini MP3** + speaker  
- **Relay Module**  
- Push Button (3 buah: Mode, Simulasi, Reset)  
- Resistor & kabel jumper  

---

## 📐 Skema Singkat
DS18B20 -> Pin 4
Relay -> Pin 5
MODE -> Pin 12
SIM -> Pin 11
RESET -> Pin 10
DFPlayer -> Pin 8 (TX)
Data Out -> Pin 9 (TX ke ESP/komputer)

---

## 🚀 Cara Kerja
1. Saat sistem dinyalakan, suhu dipantau secara berkala dan ditampilkan di LCD.  
2. Relay dikendalikan otomatis sesuai ambang batas suhu.  
3. Tombol `RESET` menandai hari awal (Start).  
4. Jika **Mode Auto**, sistem mengikuti waktu RTC. Jika **Mode Simulasi**, tombol `SIM` dipakai untuk menaikkan hari.  
5. Event akan aktif:
   - Hari ke-3 → **BALIK** (notifikasi audio + serial).  
   - Hari ke-6 → **PANEN** (notifikasi audio + serial, lalu sistem berhenti).  

---

## 📸 Tampilan LCD
- Baris 1: Suhu & jam saat ini.  
- Baris 2: Mode (Auto/Manual), nama hari, dan tanggal.  

---

## 📂 Struktur Kode
File utama: `TX_Arduino.ino dan RX_ESP32`  
Menggunakan library:
- `OneWire.h`  
- `DallasTemperature.h`  
- `LiquidCrystal_I2C.h`  
- `DFPlayer_Mini_Mp3.h`  
- `SoftwareSerial.h`  
- `RTClib.h`  

---

## ✨ Lisensi
Proyek ini bersifat open-source. Silakan digunakan, dimodifikasi, dan dikembangkan lebih lanjut.  

---

## Contacs us : 
* [Frendi RoboTech](https://www.instagram.com/frendi.co/)
* [Whatsapp : +6287888227410](https://wa.me/+6287888227410)
* [Email    : frendirobotech@gmail.com](https://mail.google.com/mail/u/0/?view=cm&tf=1&fs=1&to=frendirobotech@gmail.com) atau [Email    : frendix45@gmail.com](https://mail.google.com/mail/u/0/?view=cm&tf=1&fs=1&to=frendix45@gmail.com)

---

## 👨‍💻 Author
Dikembangkan oleh: [Reog Robotic & Robotech Electronics]  
Lisensi: Open Source (MIT)

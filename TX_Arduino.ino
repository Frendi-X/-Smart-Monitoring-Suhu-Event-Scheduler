#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <DFPlayer_Mini_Mp3.h>
#include <SoftwareSerial.h>
#include <RTClib.h>

#define ONE_WIRE_BUS 4
#define RELAY_PIN 5
#define BUTTON_MODE 12     // saklar selector mode
#define BUTTON_SIM 11      // tombol simulasi counter hari
#define BUTTON_RESET 10    // tombol reset sistem

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27, 16, 2);

SoftwareSerial DFSerial(255, 8);   // RX, TX untuk DFPlayer
SoftwareSerial dataSerial(255, 9); // RX, TX untuk Komunikasi Data

RTC_DS1307 rtc;

static String lastHari = "";
float suhuC = 0;
int lastRelayState = -1;
bool normalSent = false;

int hariSimulasi = 0;      // counter hari simulasi (manual)
int hariAwalReset = 0;     // menyimpan hari awal saat reset ditekan
bool resetAktif = false;   // status apakah sudah pernah reset ditekan

String namaHari[7] = {
  "Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"
};

String hari, waktu;
bool autoMode;
int hariRTC, selisih;

// --- Ambil waktu RTC ---
String getRTCtime() {
  char buffer[30];
  DateTime now = rtc.now();
  snprintf(buffer, sizeof(buffer), "%s, %02d-%02d-%04d %02d:%02d:%02d",
           namaHari[now.dayOfTheWeek()].c_str(),
           now.day(), now.month(), now.year(),
           now.hour(), now.minute(), now.second());
  return String(buffer);
}

// --- Ambil nama hari (RTC atau Simulasi) ---
String getHariAktif(bool autoMode) {
  if (autoMode) {
    DateTime now = rtc.now();
    return namaHari[now.dayOfTheWeek()];
  } else {
    return namaHari[hariSimulasi];
  }
}

// --- Fungsi cek event BALIK & PANEN ---
void cekEventHari(String hari, String waktu, int selisih) {
  if (!resetAktif) return; // kalau reset belum aktif, abaikan

  if (hari != lastHari) {
    Serial.print("Hari baru: ");
    Serial.print(hari);
    Serial.print(" | Selisih: ");
    Serial.println(selisih);

    if (selisih == 3) {
      Serial.println(">>> EVENT BALIK <<<");
      Serial.println("Play DFPlayer...");
      delay(50);
      mp3_play(1);
      Serial.println("Done play DFPlayer");
      delay(500);
      dataSerial.println("MSG:BALIK|TEMP:" + String(suhuC, 1) + "|TIME:" + waktu);
    }

    if (selisih == 6) {
      Serial.println(">>> EVENT PANEN <<<");

      Serial.println("Play DFPlayer...");
      delay(50);
      mp3_play(2);
      Serial.println("Done play DFPlayer");
      delay(500);  // cukup 200ms biar nggak ngeblok

      // kirim notifikasi hanya sekali
      static bool panenSent = false;
      if (!panenSent) {
        dataSerial.println("MSG:PANEN|TEMP:" + String(suhuC, 1) + "|TIME:" + waktu);
        panenSent = true;
      }

      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print(">>> STOP <<<");
      delay(3000);
      lcd.clear();

      resetAktif = false;
      Serial.println("Reset kembali ke FALSE");
    }
    lastHari = hari;
  }
}

void updateLCD() {
  // LCD update
  DateTime now = rtc.now();
  lcd.setCursor(0, 0);
  lcd.print("T:" + String(suhuC, 1) + " C ");
  lcd.setCursor(11, 0);
  lcd.print(now.hour(), DEC);
  lcd.print(":");
  lcd.print(now.minute(), DEC);
  lcd.setCursor(0, 1);
  if (autoMode) {
    lcd.print("A:" + hari + "   ");
  } else {
    lcd.print("M:" + hari + "   ");
  }
  lcd.setCursor(9, 1);
  lcd.print(now.day(), DEC);
  lcd.print("-");
  lcd.print(now.month(), DEC);
  lcd.print("-");
  lcd.print(now.year() % 100, DEC);
}


void setup() {
  Serial.begin(9600);   // komunikasi ke ESP32
  DFSerial.begin(9600); // DFPlayer
  dataSerial.begin(9600);
  mp3_set_serial(DFSerial);
  delay(1000);
  mp3_set_volume(100);

  sensors.begin();
  lcd.init();
  lcd.backlight();

  if (!rtc.begin()) {
    lcd.setCursor(0, 0);
    lcd.print("RTC ERR");
    while (1);
  }

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUTTON_MODE, INPUT_PULLUP);  // saklar mode
  pinMode(BUTTON_SIM, INPUT_PULLUP);   // push button simulasi
  pinMode(BUTTON_RESET, INPUT_PULLUP); // push button reset

  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  delay(1000);
  lcd.clear();
}

void loop() {
  sensors.requestTemperatures();
  suhuC = sensors.getTempCByIndex(0);

  // Baca saklar mode
  autoMode = (digitalRead(BUTTON_MODE) == LOW); // ditekan = AUTO
  hari = getHariAktif(autoMode);
  waktu = getRTCtime();

  int relayState = lastRelayState;

  if (suhuC < 40) {
    relayState = LOW;
    normalSent = false;  // reset flag normal
  }
  else if (suhuC > 50) {
    relayState = HIGH;
    normalSent = false;  // reset flag normal
  }
  else {
    // Suhu normal (40–50 °C)
    relayState = lastRelayState;  // relay tidak berubah
    if (!normalSent) {
      dataSerial.println("TEMP:" + String(suhuC, 1) + "|TIME:" + waktu);
      Serial.println("Notifikasi kondisi SUHU NORMAL terkirim");
      normalSent = true;  // tandai sudah terkirim sekali
    }
  }

  // Kirim notifikasi hanya jika ada perubahan relay
  if (relayState != lastRelayState) {
    digitalWrite(RELAY_PIN, relayState);
    dataSerial.println("TEMP:" + String(suhuC, 1) + "|TIME:" + waktu);
    Serial.println("Notifikasi SUHU TIDAK NORMAL terkirim");
    lastRelayState = relayState;
  }

  // --- RESET ditekan ---
  if (digitalRead(BUTTON_RESET) == LOW) {
    DateTime now = rtc.now();
    hariAwalReset = now.dayOfTheWeek();
    hariSimulasi = hariAwalReset; // sinkron juga untuk simulasi
    resetAktif = true;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(">>> START <<< ");
    lcd.setCursor(0, 1);
    lcd.print(namaHari[hariAwalReset]);
    delay(1000);
  }

  // --- Mode SIMULASI ---
  if (!autoMode) {
    if (digitalRead(BUTTON_SIM) == LOW && resetAktif) {
      hariSimulasi = (hariSimulasi + 1) % 7;
      hari = getHariAktif(autoMode);
      selisih = (hariSimulasi - hariAwalReset + 7) % 7;
      updateLCD();
      cekEventHari(hari, waktu, selisih);
    }
  }

  // --- Mode AUTO ---
  if (autoMode && resetAktif) {
    hariRTC = rtc.now().dayOfTheWeek();
    selisih = (hariRTC - hariAwalReset + 7) % 7;
    updateLCD();
    cekEventHari(hari, waktu, selisih);
  }

  updateLCD();

  delay(100);
}

#include <WiFi.h>
#include <HTTPClient.h>
#include "time.h"
#include <SoftwareSerial.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

#define RX 18
#define TX 19
SoftwareSerial mySerial(RX, TX);

// ----- WiFi & Telegram -----
const char* ssid = "Android AP";
const char* password = "12345678";
const char* telegramToken = "7800007449:AAG5hEmQVvcj_Q-ki0nlIW47rHbOqSLuDQA";          // "8468140387:AAHpVgt43AHu0o9B9BGVO8K7NE5sOylCNG4"; PUNYA FRENDI
const char* chatID = "5214575312";                                                     // "7325127591"; PUNYA FRENDI

WiFiClientSecure secured_client;
UniversalTelegramBot bot(telegramToken, secured_client);

// NTP
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 25200; // GMT+7
const int daylightOffset_sec = 0;

// ----- Suhu batas -----
#define SUHU_ON   40.0   // jika di bawah ini -> Relay ON (status)
#define SUHU_OFF  50.0  // jika di atas ini -> Relay OFF (status)

// ----- State untuk anti-spam -----
String lastMsgSent = "";    // menyimpan MSG terakhir yang sudah dikirim (BALIK/PANEN)
int lastTempState = -1;     // -1 unknown, 0 = ON (suhu rendah), 1 = OFF (suhu tinggi), 2 = NORMAL

// ----- Helper: waktu sekarang (NTP) -----
String getTimeNow() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "Waktu ERR";
  char buffer[40];
  strftime(buffer, sizeof(buffer), "%A, %d-%m-%Y %H:%M:%S", &timeinfo);
  return String(buffer);
}

// ----- Helper: format pesan (BALIK, PINDAH, PANEN, SIMULASI, default) -----
String formatPesan(String msg, String temp) {
  String judul, detail;

  if (msg.equals("BALIK")) {
    judul = "🔄 *Peringatan 3-Hari*";
    detail = "👉 Silakan *balikkan objek*.";
  }
  else if (msg.equals("PINDAH")) {
    judul = "📦 *Peringatan 5-Hari*";
    detail = "➡️ Segera *pindahkan objek*.";
  }
  else if (msg.equals("PANEN")) {
    judul = "🌾 *Peringatan 6-Hari*";
    detail = "✅ Saatnya *panen objek*.";
  }
  else if (msg.equals("SIMULASI")) {
    judul = "🧪 *Mode Simulasi*";
    detail = "⚙️ Percobaan manual dari tombol simulasi.";
  }
  else {
    judul = "ℹ️ *Notifikasi Sistem*";
    detail = "❓ Pesan tidak diketahui.";
  }

  // --- Bersihkan jika masih ada "|TIME:" di temp ---
  int pos = temp.indexOf("|TIME:");
  if (pos >= 0) {
    temp = temp.substring(0, pos);
  }

  String teks = judul + "\n\n"
                + "🕒 Waktu : " + getTimeNow() + "\n"
                + "🌡️ Suhu  : " + temp + " °C\n\n"
                + detail;

  return teks;
}


void setup() {
  Serial.begin(9600);  // komunikasi dari Arduino
  mySerial.begin(9600);
  WiFi.begin(ssid, password);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);

    Serial.println("Connecting WiFi...");
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("ESP32 Ready");
}

void loop() {
  // cek data serial dari Arduino
  if (mySerial.available()) {
    String data = mySerial.readStringUntil('\n');
    data.trim();
    if (data.length() == 0) return;

    Serial.println("Data diterima: " + data);

    // parsing sederhana: "MSG:xxx|TEMP:yy.y" atau "TEMP:yy.y"
    String msgType = "";
    String tempStr = "";
    float suhu = NAN;

    int msgPos = data.indexOf("MSG:");
    int tempPos = data.indexOf("TEMP:");

    if (msgPos >= 0) {
      int end = data.indexOf("|", msgPos);
      if (end < 0) end = data.length();
      msgType = data.substring(msgPos + 4, end);
      msgType.trim();
    }

    if (tempPos >= 0) {
      tempStr = data.substring(tempPos + 5);
      tempStr.trim();
      suhu = tempStr.toFloat();
    }

    // CASE A: ada MSG (BALIK / PANEN / PINDAH / SIMULASI) -> kirim notif sesuai MSG (sertakan suhu bila ada)
    if (msgType.length() > 0) {
      // kirim hanya sekali per MSG (anti-spam)
      if (!msgType.equals(lastMsgSent)) {
        String teks = formatPesan(msgType, (tempStr.length() ? tempStr : "N/A"));
        // gunakan Markdown (parameter ke-3 = "Markdown") supaya bold/italic muncul
        bot.sendMessage(String(chatID), teks, "Markdown");
        Serial.println("Kirim MSG notif: " + msgType + " | " + teks);
        lastMsgSent = msgType;
      } else {
        Serial.println("MSG sama dengan terakhir, skip kirim.");
      }
      // reset suhu state (optional) supaya notif suhu bisa muncul ketika berubah setelah event
      // lastTempState = -1;
    }
    // CASE B: tidak ada MSG, hanya TEMP -> kirim status suhu jika berubah (ON/OFF/normal)
    else if (tempStr.length() > 0) {
      int curState;
      if (suhu < SUHU_ON) curState = 0;       // Relay ON (suhu rendah)
      else if (suhu > SUHU_OFF) curState = 1; // Relay OFF (suhu tinggi)
      else curState = 2;                      // normal

      if (curState != lastTempState) {
        String statusText;
        if (curState == 0) statusText = "Relay ON (suhu " + String(suhu, 1) + " °C)";
        else if (curState == 1) statusText = "Relay OFF (suhu " + String(suhu, 1) + " °C)";
        else statusText = "Suhu normal: " + String(suhu, 1) + " °C";

        String pesan = "*Update Suhu* \n\nWaktu : " + getTimeNow() + "\n" + "Suhu  : " + String(suhu, 1) + " °C\n" + statusText;

        bot.sendMessage(String(chatID), pesan, "Markdown");
        Serial.println("Kirim TEMP notif: " + pesan);
        lastTempState = curState;
      } else {
        Serial.println("Status suhu sama, skip kirim.");
      }
    }
  }

  // singgah singkat agar loop tidak terlalu berat
  delay(50);
}

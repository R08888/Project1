//R08888
#define BLYNK_TEMPLATE_ID "TMPLv5vCaiaj"
#define BLYNK_TEMPLATE_NAME "Control relay"
#define BLYNK_AUTH_TOKEN "WRHcYcbbUZ1AiAcof-A3cioPzJ9Ufeib"

#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <DHT.h>
#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp32.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <DHT.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // LCD lokal
#define DHTPIN 4                     // What digital pin we're connected to
#define DHTTYPE DHT22                // DHT 22, AM2302, AM2321
DHT dht(DHTPIN, DHTTYPE);

WidgetLCD lcd2(V9);  //Blynk LCD

#define RLed 33 //Lampu
#define BLed 25 //Misting

float h, t;

float calibratedHumidity;
float calibratedTemperature;

int otomatisBlynk, relayBlynk_1, relayBlynk_2;

const int RELAY_PIN_1 = 14;  //Lampu Halogen/Halogen Lamp
const int RELAY_PIN_2 = 27;  //Pompa Air/Water Pump

//SDA pin on I2C module to pin D21 on ESP32
//SCL pin on I2C module to pin D22 on ESP32

const float MinTemperature = 26;  //Set Point untuk Suhu batas Bawah
const float MaxTemperature = 27;  //Set Point untuk Suhu batas Atas

const float MinKelembapan = 65;
const float MaxKelembapan = 66;

BlynkTimer timer;  //Timer untuk menjalankan fungsi secara berkala

char ssid[] = "TexasKost4";     // Your WiFi SSID
char pass[] = "wisnuudahpwt";  // Paswword SSID/WiFi

void setup() {
  // Debug console
  Serial.begin(115200);
  lcd.begin();
  lcd.backlight();

  // Mulai koneksi WiFi
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Menghubungkan ");
  lcd.setCursor(0, 1);
  lcd.print(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.setCursor(13, 0);
    lcd.print("..");
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi terhubung");

  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());

  // Mulai koneksi ke Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  dht.begin();

  pinMode(RELAY_PIN_1, OUTPUT);
  pinMode(RELAY_PIN_2, OUTPUT);

  pinMode(RLed, OUTPUT);
  pinMode(BLed, OUTPUT);

  // Setup a function to be called every second
  timer.setInterval(2000L, sendSensor);
  timer.setInterval(2000L, autocontrol);
  timer.setInterval(2000L, notifikasi);
}

void loop() {
  while (WiFi.status() != WL_CONNECTED) {
    otomatiswifi();
  }
  timer.run();
  Blynk.run();
}

void autocontrol() {
  if (otomatisBlynk == 1) {

    lcd2.print(6, 1, "Otomat");
    lcd.setCursor(10, 1);
    lcd.print("Otomat");

    if (calibratedHumidity <= MinKelembapan) {
      digitalWrite(RELAY_PIN_2, LOW);  //Turn on Relay 2/Misting
      digitalWrite(RELAY_PIN_1, HIGH);
      Blynk.virtualWrite(V3, 0);
      Blynk.virtualWrite(V2, 1);
      digitalWrite(BLed, HIGH);  //Turn on Red Led
      digitalWrite(RLed, LOW);   //Turn on Red Led
    }

    else if (calibratedHumidity >= MaxKelembapan) {
      digitalWrite(RELAY_PIN_2, HIGH);
      Blynk.virtualWrite(V3, 1);
      digitalWrite(BLed, LOW);   //Turn on Red Led
    }

    if (calibratedTemperature <= MinTemperature) {
      digitalWrite(RELAY_PIN_1, LOW);  //Turn on Relay 1/Lampu
      digitalWrite(RELAY_PIN_2, HIGH);
      Blynk.virtualWrite(V2, 0);
      Blynk.virtualWrite(V3, 1);
      digitalWrite(RLed, HIGH);  //Turn on Red Led
      digitalWrite(BLed, LOW);   //Turn on Red Led
    }

    else if (calibratedTemperature >= MaxTemperature) {
      digitalWrite(RELAY_PIN_1, HIGH);  //Turn off Relay 1/Lampu
      Blynk.virtualWrite(V2, 1);
      digitalWrite(RLed, LOW);  //Turn on Red Led
    }
  }
}

/*-----------------------------------------------------------------------------------------*/

BLYNK_CONNECTED() {
  // Request the latest state from the server
  Blynk.syncVirtual(V2); //lampu
  Blynk.syncVirtual(V3); //misting
  Blynk.syncVirtual(V4); //otomatis
}


BLYNK_WRITE(V2) {  // Lampu
  relayBlynk_1 = param.asInt();
  digitalWrite(RELAY_PIN_1, relayBlynk_1);
}

BLYNK_WRITE(V3) {  // Pompa
  relayBlynk_2 = param.asInt();
  digitalWrite(RELAY_PIN_2, relayBlynk_2);
}

BLYNK_WRITE(V4) {  // BLYNK_WRITE callback untuk Virtual Pin 4
  otomatisBlynk = param.asInt();
}

/*--------------------------------------------------------------------------------------*/

void sendSensor() {

  h = dht.readHumidity();
  t = dht.readTemperature();  // or dht.readTemperature(true) for Fahrenheit

  calibratedHumidity = h - 31;
  calibratedTemperature = t + 2;

  if (isnan(h) || isnan(t)) {
    lcd2.clear();
    lcd2.print(0, 0, "Gagal membaca");  // Sending to LCD Blynk "Failed to read data from sensor"
    lcd2.print(0, 1, "sensor");         // Sending to LCD Blynk "Failed to read data from sensor"

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Gagal membaca");  // Sending to LCD I2C "Failed to read data from sensor"

    lcd.setCursor(0, 1);
    lcd.print("sensor");  // Sending to LCD I2C "Failed to read data from sensor"

    delay(1000);
    return;
  }

  Blynk.virtualWrite(V5, calibratedHumidity);
  Blynk.virtualWrite(V6, calibratedTemperature);

  lcd.clear();
  lcd.setCursor(0, 0);  //menentukan posisi kalimat pada LCD
  lcd.print("H: ");
  lcd.print(calibratedHumidity);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("T: ");
  lcd.print(calibratedTemperature);
  lcd.print("C");

  lcd.setCursor(10, 0);
  lcd.print("Mode:");

  lcd2.print(0, 1, "Mode:");
}

void notifikasi() {

  int kondisiRelay1 = digitalRead(RELAY_PIN_1);
  int kondisiRelay2 = digitalRead(RELAY_PIN_2);

  if (t <= MinTemperature) {
    lcd2.print(0, 0, "Suhu Dinginn!");  //Sending " Cold Temperature" To Blynk
  }

  else if (t >= MaxTemperature) {
    lcd2.print(0, 0, "Suhu Panass!");  //Sending " Hot Temperature" To Blynk
  }

  else if (t > MinTemperature && t < MaxTemperature) {
    lcd2.print(0, 0, "Suhu Amann!");  //Sending " Temperature is good " To Blynk
  }

  if (otomatisBlynk == 0) {
    lcd2.print(6, 1, "Manual");

    lcd.setCursor(10, 1);
    lcd.print("Manual");

    if (kondisiRelay1 == LOW) {
      digitalWrite(RLed, HIGH);  //Turn On Red Led
    }

    else if (kondisiRelay1 == HIGH) {
      digitalWrite(RLed, LOW);  //Turn Off Red Led
    }

    if (kondisiRelay2 == LOW) {
      digitalWrite(BLed, HIGH);  //Turn On Blue Led

    } else if (kondisiRelay2 == HIGH) {
      digitalWrite(BLed, LOW);  //Turn Off Blue Led
    }
  }
}

void otomatiswifi() {
  // Cek apakah koneksi WiFi dan Blynk terhubung
  if (WiFi.status() != WL_CONNECTED || !Blynk.connected()) {

    // Coba menghubungkan ulang ke WiFi
    while (WiFi.status() != WL_CONNECTED) {

      // Mulai koneksi WiFi
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Menghubungkan");
      lcd.setCursor(0, 1);
      lcd.print(ssid);

      WiFi.begin(ssid, pass);

      while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        lcd.setCursor(13, 0);
        lcd.print("..");
      }

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("WiFi terhubung");

      lcd.setCursor(0, 1);
      lcd.print(WiFi.localIP());
      lcd.clear();

      // Mulai koneksi ke Blynk
      Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    }
  }
}

#include <Wire.h>
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <WiFiClient.h>
#include <ThingSpeak.h> // Include ThingSpeak library

const char *ssid = "Nothing phone(1)";
const char *password = "6191yuup";
#define SENSOR_PIN A0
LiquidCrystal_I2C lcd(0x27, 16, 2);
const int sampleWindow = 50;
unsigned int sample;
int db;
WiFiClient client;
unsigned long myChannelNumber = 2453206; // Removed quotes to make it an unsigned long
const char *myWriteAPIKey = "5KQWWKJ18TF3OUYD";

void setup() {
  pinMode(SENSOR_PIN, INPUT);
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  // Connect to the WiFi network
  WiFi.begin(ssid, password);
  ThingSpeak.begin(client);
}

void loop() {
  unsigned long startMillis = millis(); // Start of sample window
  float peakToPeak = 0;                // peak-to-peak level
  unsigned int signalMax = 0;          // minimum value
  unsigned int signalMin = 1024;       // maximum value

  // Collect data for 50 mS
  while (millis() - startMillis < sampleWindow) {
    sample = analogRead(SENSOR_PIN); // Get reading from the microphone

    if (sample < 1024) // Toss out spurious readings
    {
      if (sample > signalMax) {
        signalMax = sample; // Save just the max levels
      } else if (sample < signalMin) {
        signalMin = sample; // Save just the min levels
      }
    }
  }

  peakToPeak = signalMax - signalMin; // Max - min = peak-peak amplitude
  Serial.println(peakToPeak);

  db = map(peakToPeak, 20, 900, 49.5, 90); // Calibrate for deciBels

  lcd.setCursor(0, 0);
  lcd.print("Loudness: ");
  lcd.print(db);
  lcd.print("dB");

  if (db <= 50) {
    lcd.setCursor(0, 1);
    lcd.print("Level: Quiet");
  } else if (db > 50 && db < 75) {
    lcd.setCursor(0, 1);
    lcd.print("Level: Moderate");
  } else if (db >= 75) {
    lcd.setCursor(0, 1);
    lcd.print("Level: High");
  }

  // Update ThingSpeak channel with the loudness value
  ThingSpeak.writeField(myChannelNumber, 1, db, myWriteAPIKey);

  delay(600);
  lcd.clear();
}

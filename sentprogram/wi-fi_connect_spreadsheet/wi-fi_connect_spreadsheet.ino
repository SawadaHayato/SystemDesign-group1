#include <M5Stack.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Wi-Fi設定
const char* ssid = "sawasakuraのiPhone";
const char* password = "swkg6y9xwnhp0";

// ★ Apps ScriptのWebアプリURL
const char* scriptURL = "https://script.google.com/macros/s/AKfycbyI2E1DhC4VxDrSHS7T3qiPjYEC9tpqT8PilYEBz8Fv5v99lWSrLTZ33RN4PVrJBAYG/exec";

void setup() {
  M5.begin();
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, 0);

  M5.Lcd.println("WiFi connecting...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    M5.Lcd.print(".");
  }

  M5.Lcd.println("\nConnected!");
}

void loop() {
  M5.update();

  if (M5.BtnA.wasPressed()) {
    M5.Lcd.println("Button A!");

    HTTPClient http;
    http.begin(scriptURL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int httpCode = http.POST("count=1");

    if (httpCode > 0) {
      M5.Lcd.println("Sent to Sheet");
    } else {
      M5.Lcd.println("Send failed");
    }

    http.end();
  }

  delay(10);
}

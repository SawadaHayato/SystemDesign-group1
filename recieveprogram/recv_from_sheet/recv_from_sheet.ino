#include <M5Stack.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

const char* ssid = "Galaxy_5GMW_2519";
const char* password = "eerr0449";
const char* scriptURL = "https://script.google.com/macros/s/AKfycbySw8R1D5C2Bbse0Th5JOkmtVnZ4ukdR7BU9wI2INr3jdT_refTnvoGPy7_-OGwrhep/exec";

const unsigned long POLL_MS = 2000;
unsigned long lastPoll = 0;

int extractInt(const String& json, const char* key) {
  String pat = String("\"") + key + "\":";
  int i = json.indexOf(pat);
  if (i < 0) return -1;
  i += pat.length();
  while (i < (int)json.length() && json[i] == ' ') i++;
  int j = i;
  if (j < (int)json.length() && json[j] == '-') j++;
  while (j < (int)json.length() && isDigit(json[j])) j++;
  if (j == i) return -1;
  return json.substring(i, j).toInt();
}

void setup() {
  M5.begin();
  M5.Lcd.setBrightness(200);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, 0);

  M5.Lcd.println("WiFi connecting...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    M5.Lcd.print(".");
  }
  M5.Lcd.println("\nWiFi OK");
}

void loop() {
  unsigned long now = millis();
  if (now - lastPoll >= POLL_MS) {
    lastPoll = now;

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.begin(client, scriptURL);

    int code = http.GET();
    String body = (code > 0) ? http.getString() : "";
    http.end();

    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("HTTP: %d\n\n", code);

    if (code == 200) {
      body.trim();
      int a = extractInt(body, "A");
      int b = extractInt(body, "B");
      int c = extractInt(body, "C");

      M5.Lcd.printf("A: %d\n", a);
      M5.Lcd.printf("B: %d\n", b);
      M5.Lcd.printf("C: %d\n", c);
    } else {
      M5.Lcd.println("GET failed");
      if (body.length() > 0) {
        if (body.length() > 120) body = body.substring(0, 120);
        M5.Lcd.println(body);
      }
    }
  }

  delay(10);
}

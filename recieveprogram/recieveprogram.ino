#include <M5Stack.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// Wi-Fi設定
// 以下はつながらないので、一旦使わない
//const char* ssid = "Galaxy_5GMW_2519";
//const char* password = "eerr0449";

// 以下の三井のテザリングで接続する
const char* ssid = "test";
const char* password = "testtest";
const char* scriptURL = "https://script.google.com/macros/s/AKfycbySw8R1D5C2Bbse0Th5JOkmtVnZ4ukdR7BU9wI2INr3jdT_refTnvoGPy7_-OGwrhep/exec";

const unsigned long POLL_MS = 2000;
unsigned long lastPoll = 0;

// ボタンプログラム用
int current_threshold_A = 30;
int current_threshold_B = 1;
int a = 0;
int b = 0;

bool flashflag(int ButtonSum, int current_threshold) {
  return ButtonSum >= current_threshold;
}

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

  // Aボタンの判定
  if (flashflag(a, current_threshold_A)) {
    showCenterText("red", TFT_RED);
    current_threshold_A += a; 
  }
  // Bボタンの判定
  if (flashflag(b, current_threshold_B)) {
    showCenterText("blue", TFT_BLUE);
    current_threshold_B += b;
  }

  delay(10);
}

// 中央に文字を表示するための専用関数
void showCenterText(const char* text, uint16_t color) {
  M5.Lcd.clear();
  M5.Lcd.setTextColor(color);
  
  // 画面の中心（160, 120）にテキストを表示
  M5.Lcd.drawString(text, 160, 120);
  
  delay(5000); // 5秒待機
  M5.Lcd.clear();
}
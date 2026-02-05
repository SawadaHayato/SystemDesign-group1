#include <M5Stack.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// --- 設定 ---
const char* ssid = "Galaxy_5GMW_2519";
const char* password = "eerr0449";
const char* scriptURL = "https://script.google.com/macros/s/AKfycbySw8R1D5C2Bbse0Th5JOkmtVnZ4ukdR7BU9wI2INr3jdT_refTnvoGPy7_-OGwrhep/exec";

const unsigned long POLL_MS = 2000;
unsigned long lastPoll = 0;

// ロジック用変数：閾値をそれぞれ用意
int thresholdA = 10;
int thresholdB = 30;
int AButtonSum = 0;
int BButtonSum = 0;

// --- 関数定義 ---

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

void showCenterText(const char* text, uint16_t color) {
  M5.Lcd.clear();
  M5.Lcd.setTextColor(color);
  M5.Lcd.setTextSize(4); 
  M5.Lcd.setTextDatum(MC_DATUM); 
  M5.Lcd.drawString(text, 160, 120);
  delay(5000); 
  M5.Lcd.clear();
  M5.Lcd.setTextSize(2); 
}

// 判定用の関数：今の数値が今の閾値以上かチェック
bool isOverThreshold(int currentVal, int targetThreshold) {
  return currentVal >= targetThreshold;
}

// --- メイン処理 ---

void setup() {
  M5.begin();
  M5.Lcd.setBrightness(200);
  M5.Lcd.setTextSize(2);
  
  M5.Lcd.println("WiFi connecting...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    M5.Lcd.print(".");
  }
  M5.Lcd.println("\nWiFi OK");
  delay(1000);
}

void loop() {
  M5.update();
  unsigned long now = millis();

  if (now - lastPoll >= POLL_MS) {
    lastPoll = now;

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.begin(client, scriptURL);

    int code = http.GET();
    if (code == 200) {
      String body = http.getString();
      body.trim();

      AButtonSum = extractInt(body, "A");
      BButtonSum = extractInt(body, "B");

      // 通常時のステータス表示
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.setTextDatum(TL_DATUM); 
      M5.Lcd.setTextColor(WHITE);
      
      M5.Lcd.printf("A: %d (Thres: %d)\n", AButtonSum, thresholdA);
      M5.Lcd.printf("B: %d (Thres: %d)\n", BButtonSum, thresholdB);

      // --- 判定ロジック (A) ---
      if (isOverThreshold(AButtonSum, thresholdA)) {
        showCenterText("RED ALERT", TFT_RED);
        thresholdA += 10; // Aの閾値だけを更新
      }
      
      // --- 判定ロジック (B) ---
      if (isOverThreshold(BButtonSum, thresholdB)) {
        showCenterText("BLUE ALERT", TFT_BLUE);
        thresholdB += 10; // Bの閾値だけを更新
      }
      
    } else {
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.printf("HTTP Error: %d\n", code);
    }
    http.end();
  }
  delay(10);
}
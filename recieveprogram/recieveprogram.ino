#include <M5Stack.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <stdlib.h>
// Wi-Fi設定
// 以下はつながらないので、一旦使わない
//const char* ssid = "Galaxy_5GMW_2519";
//const char* password = "eerr0449";

// 以下の三井のテザリングで接続する
const char* ssid = "test";
const char* password = "testtest";
const char* scriptURL = "https://script.google.com/macros/s/AKfycbz2lFhXlfZFqy6cph4DF-oYbcWMxCozfQYZJ9jNd9mNj9diKGw3EQzrP74D39A4Bn2D/exec";

const unsigned long POLL_MS = 2000;
unsigned long lastPoll = 0;

// ボタンプログラム用
int current_threshold_A = 5;
int current_threshold_B = 1;
int current_threshold_C = 1;

int a = 0;
int b = 0;
int c = 0;

bool used[10] = {false}; // 既読フラグ（すべて未読で初期化）
int usedCount = 0;       // 何個表示したかをカウント

char topic[10][10] = {"Hobby", "Food", "Music", "Movie", "Travel", "Sports", "Dream", "Weekend", "Challenge", "Routine"};
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
  
  WiFi.begin(ssid, password);
  M5.Lcd.print("Connecting to WiFi");
  //変更箇所
  WiFiClientSecure client;
  client.setInsecure(); // SSL証明書の検証をスキップ
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    M5.Lcd.print(".");
    retry++;
  }
  
  // --- ここから追加：起動時にGASの値をリセットする ---
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    // URLに ?action=reset を付与してリセット命令を送る
    String resetURL = String(scriptURL) + "?action=reset";
    http.begin(resetURL);
    int httpCode = http.POST(""); // 空のデータをPOST
    
    if (httpCode > 0) {
      M5.Lcd.println("\nSystem Reset OK");
    } else {
      M5.Lcd.println("\nReset Failed");
    }
    http.end();
    delay(5000); // 確認用に少し待機
  }
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
      a = extractInt(body, "A");
      b = extractInt(body, "B");
      c = extractInt(body, "C");

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
    showCenterText("ORANGE", TFT_ORANGE);
    current_threshold_A += a - current_threshold_A; 
  }
  // Bボタンの判定
  if (flashflag(b, current_threshold_B)) {
    showCenterText("BLUE", TFT_BLUE);
    current_threshold_B += b - current_threshold_B;
  }
  // Cボタンの判定
  if (flashflag(c, current_threshold_C)) {
    // 全て使い切っていたらリセット（ループ再生したい場合）
    if (usedCount >= 10) {
        for (int i = 0; i < 10; i++) used[i] = false;
        usedCount = 0;
    }

    int num;
    // まだ使われていない番号が出るまで繰り返す
    do {
        num = rand() % 10;
    } while (used[num] == true);

    // 選ばれた番号を「使用済み」にする
    used[num] = true;
    usedCount++;

    showCenterText(topic[num], TFT_GREEN);
    current_threshold_C += c - current_threshold_C;
  }

  delay(10);
}

// 中央に文字を表示するための専用関数
void showCenterText(const char* text, uint16_t color) {
  M5.Lcd.clear();
  M5.Lcd.setTextColor(color);
  M5.Lcd.setTextSize(6);
  // 画面の中心（160, 120）にテキストを表示
  M5.Lcd.drawString(text, 80, 120);
  M5.Lcd.setTextSize(2);
  
  delay(5000); // 5秒待機
  M5.Lcd.clear();
}
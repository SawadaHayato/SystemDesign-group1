#include <M5Stack.h>
#include <WiFi.h>
#include <HTTPClient.h>

// --- 設定エリア ---
const char* ssid = "sawasakuraのiPhone";
const char* password = "swkg6y9xwnhp0";
const char* scriptURL = "https://script.google.com/macros/s/AKfycbyI2E1DhC4VxDrSHS7T3qiPjYEC9tpqT8PilYEBz8Fv5v99lWSrLTZ33RN4PVrJBAYG/exec";

const uint32_t THRESHOLD = 20;        // 閾値：20回
const unsigned long LIMIT_MS = 10000; // 制限時間：10秒

// --- 変数管理 ---
uint32_t currentIntervalA = 0;
uint32_t currentIntervalB = 0;
uint32_t currentIntervalC = 0;
unsigned long lastIntervalStart = 0;

// --- 関数定義 ---

// メインの黒い画面（メニュー）を描画
void drawStaticMenu() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.println("Hybrid Send Mode");
  M5.Lcd.setCursor(10, 40);
  M5.Lcd.printf("Threshold:%d / Limit:10s", THRESHOLD);
}

// カウントとタイマーをリセット
void resetAll() {
  currentIntervalA = 0;
  currentIntervalB = 0;
  currentIntervalC = 0;
  lastIntervalStart = millis();
}

// 送信処理（成功・失敗・WiFiエラーすべてから自動復帰）
void sendData(int count, char btn, String typeText) {
  // 1. WiFi未接続チェック
  if (WiFi.status() != WL_CONNECTED) {
    M5.Lcd.fillScreen(BLUE); // エラー時は青
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(20, 100);
    M5.Lcd.println("WiFi ERROR");
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(20, 140);
    M5.Lcd.println("Return to Menu...");
    
    delay(2000);        // 2秒表示して
    drawStaticMenu();   // 黒画面に戻る
    return;
  }

  HTTPClient http;
  http.begin(scriptURL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String payload = "count=" + String(count) + "&btn=" + String(btn);
  int httpCode = http.POST(payload);
  
  if (httpCode > 0) {
    // 2. 送信成功
    M5.Lcd.fillScreen(GREEN);
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(20, 70);
    M5.Lcd.printf("%s SEND!", typeText.c_str());
    M5.Lcd.setCursor(20, 120);
    M5.Lcd.setTextSize(4);
    M5.Lcd.printf("Btn %c: %d", btn, count);
    
    delay(2000); 
    drawStaticMenu();
  } else {
    // 3. HTTPエラー（サーバー側の問題など）
    M5.Lcd.fillScreen(RED);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(40, 100);
    M5.Lcd.println("HTTP FAILED");
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(40, 140);
    M5.Lcd.printf("Error: %d", httpCode);

    delay(2000);
    drawStaticMenu();
  }
  http.end();
}

void setup() {
  M5.begin();
  
  // 起動時にWiFi接続を開始
  WiFi.begin(ssid, password);
  M5.Lcd.print("Connecting to WiFi");
  
  // つながるまで最大10秒待機
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    M5.Lcd.print(".");
    retry++;
  }
  
  drawStaticMenu();
  resetAll();
}

void loop() {
  M5.update();
  unsigned long now = millis();

  // --- 1) ボタン入力と閾値判定 (20回で即送信) ---
  if (M5.BtnA.wasPressed()) {
    currentIntervalA++;
    if (currentIntervalA >= THRESHOLD) {
      sendData(currentIntervalA, 'A', "QUICK");
      resetAll();
      return; 
    }
  }
  if (M5.BtnB.wasPressed()) {
    currentIntervalB++;
    if (currentIntervalB >= THRESHOLD) {
      sendData(currentIntervalB, 'B', "QUICK");
      resetAll();
      return;
    }
  }
  if (M5.BtnC.wasPressed()) {
    currentIntervalC++;
    if (currentIntervalC >= THRESHOLD) {
      sendData(currentIntervalC, 'C', "QUICK");
      resetAll();
      return;
    }
  }

  // --- 2) 制限時間を超えた時の処理 (届かなくても送る) ---
  if (now - lastIntervalStart >= LIMIT_MS) {
    // どれか一つでもカウントがあれば送信を実行
    if (currentIntervalA > 0) sendData(currentIntervalA, 'A', "TIME UP");
    if (currentIntervalB > 0) sendData(currentIntervalB, 'B', "TIME UP");
    if (currentIntervalC > 0) sendData(currentIntervalC, 'C', "TIME UP");

    resetAll();
  }

  // --- 3) 画面表示の更新 (現在のカウントと残り時間) ---
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 100);
  M5.Lcd.setTextColor(CYAN, BLACK);
  M5.Lcd.printf("A:%2d  B:%2d  C:%2d", currentIntervalA, currentIntervalB, currentIntervalC);
  
  long remaining = (LIMIT_MS - (now - lastIntervalStart)) / 100;
  if (remaining < 0) remaining = 0;
  M5.Lcd.setCursor(10, 130);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.printf("Time: %2ld.%ld s ", remaining / 10, remaining % 10);

  delay(10);
}
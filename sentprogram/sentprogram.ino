
#include <M5Stack.h>
#include <WiFi.h>
#include <HTTPClient.h>

// --- 設定エリア ---
const char* ssid = "sawasakuraのiPhone";
const char* password = "swkg6y9xwnhp0";
const char* scriptURL = "https://script.google.com/macros/s/AKfycbyI2E1DhC4VxDrSHS7T3qiPjYEC9tpqT8PilYEBz8Fv5v99lWSrLTZ33RN4PVrJBAYG/exec";

const uint32_t THRESHOLD = 20;        // 20回で即送信
const unsigned long LIMIT_MS = 10000; // 10秒制限

// --- 変数管理 ---
uint32_t currentIntervalA = 0;
uint32_t currentIntervalB = 0;
uint32_t currentIntervalC = 0;
unsigned long lastIntervalStart = 0;

// UI表示・ポップアップ制御
bool popupVisible = false;
unsigned long popupShownAt = 0;
const unsigned long POPUP_MS = 1500;
bool lastWiFiConnected = false;

// --- 関数定義 ---

// 1つ目のコード風のUIメニュー描画
void drawStaticMenu() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setTextSize(2);

  M5.Lcd.setCursor(10, 10);
  M5.Lcd.println("Hybrid Send UI");

  M5.Lcd.setCursor(10, 40);
  M5.Lcd.println("A = Ehh...");
  M5.Lcd.println("B = Okay.");
  M5.Lcd.println("C = Nice!");

  M5.Lcd.setCursor(10, 105);
  M5.Lcd.printf("Target: %d taps / 10s", THRESHOLD);
}

// 1つ目のコードにあるWiFiステータス表示
void drawWiFiStatus(bool connected) {
  M5.Lcd.fillRect(10, 130, 300, 30, BLACK);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 130);
  M5.Lcd.print("WiFi: ");
  M5.Lcd.println(connected ? "Connected" : "Not connected");
  lastWiFiConnected = connected;
}

// 1つ目のコードにあるカウント表示
void drawCountsLine() {
  M5.Lcd.fillRect(10, 160, 300, 30, BLACK);
  M5.Lcd.setTextColor(CYAN, BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 160);
  M5.Lcd.printf("A:%2d  B:%2d  C:%2d", currentIntervalA, currentIntervalB, currentIntervalC);
}

// 1つ目のコードにあるポップアップ通知
void showPopup(const char* line1, const char* line2) {
  M5.Lcd.fillRect(10, 195, 300, 45, BLACK);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 195);
  M5.Lcd.print(line1);
  M5.Lcd.setCursor(10, 217);
  M5.Lcd.print(line2);
  popupVisible = true;
  popupShownAt = millis();
}

void clearPopup() {
  M5.Lcd.fillRect(10, 195, 300, 45, BLACK);
  popupVisible = false;
}

// カウントとタイマーをリセット
void resetAll() {
  currentIntervalA = 0;
  currentIntervalB = 0;
  currentIntervalC = 0;
  lastIntervalStart = millis();
  drawCountsLine();
}

// 2つ目のコードのロジック（送信・演出）
void sendData(int count, char btn, String typeText) {
  if (WiFi.status() != WL_CONNECTED) {
    showPopup("WiFi Error", "Check Connection");
    M5.Lcd.fillScreen(BLUE);
    delay(2000);
    drawStaticMenu();
    drawWiFiStatus(false);
    return;
  }

  HTTPClient http;
  http.begin(scriptURL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String payload = "count=" + String(count) + "&btn=" + String(btn);
  int httpCode = http.POST(payload);
  
  if (httpCode > 0) {
    // 成功演出（2つ目のコードの機能）
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
    drawWiFiStatus(true);
  } else {
    // 失敗演出
    M5.Lcd.fillScreen(RED);
    delay(2000);
    drawStaticMenu();
    drawWiFiStatus(true);
  }
  http.end();
}

void setup() {
  M5.begin();
  
  WiFi.begin(ssid, password);
  drawStaticMenu();
  
  // WiFi接続待機
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 10) {
    delay(500);
    retry++;
  }
  
  drawWiFiStatus(WiFi.status() == WL_CONNECTED);
  resetAll();
}

void loop() {
  M5.update();
  unsigned long now = millis();

  // --- 1) ボタン入力と即時判定 (2つ目のコードのロジック) ---
  if (M5.BtnA.wasPressed()) {
    currentIntervalA++;
    drawCountsLine();
    if (currentIntervalA >= THRESHOLD) {
      sendData(currentIntervalA, 'A', "QUICK");
      resetAll();
      return; 
    }
  }
  if (M5.BtnB.wasPressed()) {
    currentIntervalB++;
    drawCountsLine();
    if (currentIntervalB >= THRESHOLD) {
      sendData(currentIntervalB, 'B', "QUICK");
      resetAll();
      return;
    }
  }
  if (M5.BtnC.wasPressed()) {
    currentIntervalC++;
    drawCountsLine();
    if (currentIntervalC >= THRESHOLD) {
      sendData(currentIntervalC, 'C', "QUICK");
      resetAll();
      return;
    }
  }

  // --- 2) 制限時間切れによる自動送信 (2つ目のコードのロジック) ---
  if (now - lastIntervalStart >= LIMIT_MS) {
    if (currentIntervalA > 0) sendData(currentIntervalA, 'A', "TIME UP");
    if (currentIntervalB > 0) sendData(currentIntervalB, 'B', "TIME UP");
    if (currentIntervalC > 0) sendData(currentIntervalC, 'C', "TIME UP");
    resetAll();
  }

  // --- 3) WiFiステータス監視とポップアップ更新 (1つ目のコードのUI要素) ---
  bool connected = (WiFi.status() == WL_CONNECTED);
  if (connected != lastWiFiConnected) {
    drawWiFiStatus(connected);
  }

  // 残り時間の表示更新
  long remaining = (LIMIT_MS - (now - lastIntervalStart)) / 100;
  if (remaining < 0) remaining = 0;
  M5.Lcd.setCursor(200, 105);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.printf("%2ld.%ld s ", remaining / 10, remaining % 10);

  // ポップアップ消去処理
  if (popupVisible && (now - popupShownAt >= POPUP_MS)) {
    clearPopup();
  }

  delay(10);
}
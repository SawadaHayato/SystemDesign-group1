#include <M5Stack.h>
#include <WiFi.h>
#include <HTTPClient.h>

// --- 設定エリア ---
const char* ssid = "sawasakuraのiPhone";
const char* password = "swkg6y9xwnhp0";
const char* scriptURL = "https://script.google.com/macros/s/AKfycbyI2E1DhC4VxDrSHS7T3qiPjYEC9tpqT8PilYEBz8Fv5v99lWSrLTZ33RN4PVrJBAYG/exec";

const uint32_t THRESHOLD = 20;        // 20回で送信
const unsigned long LIMIT_MS = 10000; // 10秒制限

// --- 変数管理 ---
uint32_t currentIntervalA = 0;
uint32_t currentIntervalB = 0;
uint32_t currentIntervalC = 0;
unsigned long lastIntervalStart = 0;

// UI表示用
bool popupVisible = false;
unsigned long popupShownAt = 0;
const unsigned long POPUP_MS = 1500;

// --- 関数定義 ---

// 最初の画面表示
void drawStaticMenu() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.println("Speed Clicker");
  M5.Lcd.setCursor(10, 40);
  M5.Lcd.printf("Target: %d hits in 10s", THRESHOLD);
}

// 簡易通知
void showPopup(const char* line1, const char* line2) {
  M5.Lcd.fillRect(10, 185, 300, 50, BLACK);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setCursor(10, 185);
  M5.Lcd.print(line1);
  M5.Lcd.setCursor(10, 207);
  M5.Lcd.print(line2);
  popupVisible = true;
  popupShownAt = millis();
}

// データ送信関数（成功・失敗時に大きく表示）
void sendData(int count, char btn) {
  if (WiFi.status() != WL_CONNECTED) {
    showPopup("WiFi Error", "Check Connection");
    delay(1000);
    return;
  }

  HTTPClient http;
  http.begin(scriptURL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String payload = "count=" + String(count) + "&btn=" + String(btn);
  int httpCode = http.POST(payload);
  
  if (httpCode > 0) {
    // 【成功】3秒間 緑の画面で大きく表示
    M5.Lcd.fillScreen(GREEN);
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.setTextSize(4);
    M5.Lcd.setCursor(50, 60);
    M5.Lcd.print("SUCCESS!");
    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(40, 120);
    M5.Lcd.printf("Button %c Sent", btn);
    M5.Lcd.setCursor(40, 160);
    M5.Lcd.printf("Count: %d", count);
    
    delay(3000); 
    drawStaticMenu();
  } else {
    // 【失敗】3秒間 赤い画面
    M5.Lcd.fillScreen(RED);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(40, 100);
    M5.Lcd.println("SEND FAILED");
    delay(3000);
    drawStaticMenu();
  }
  http.end();
}

void resetAll() {
  currentIntervalA = 0;
  currentIntervalB = 0;
  currentIntervalC = 0;
  lastIntervalStart = millis();
}

void setup() {
  M5.begin();
  WiFi.begin(ssid, password);
  drawStaticMenu();
  resetAll();
}

void loop() {
  M5.update();
  unsigned long now = millis();

  // 10秒経過したらリセット
  if (now - lastIntervalStart >= LIMIT_MS) {
    resetAll();
  }

  // ボタン判定（20回超えたら即送信）
  if (M5.BtnA.wasPressed()) {
    currentIntervalA++;
    if (currentIntervalA >= THRESHOLD) {
      sendData(1, 'A');
      resetAll();
    }
  }
  if (M5.BtnB.wasPressed()) {
    currentIntervalB++;
    if (currentIntervalB >= THRESHOLD) {
      sendData(1, 'B');
      resetAll();
    }
  }
  if (M5.BtnC.wasPressed()) {
    currentIntervalC++;
    if (currentIntervalC >= THRESHOLD) {
      sendData(1, 'C');
      resetAll();
    }
  }

  // 画面のカウント更新
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 100);
  M5.Lcd.setTextColor(CYAN, BLACK);
  M5.Lcd.printf("A:%2d  B:%2d  C:%2d", currentIntervalA, currentIntervalB, currentIntervalC);
  
  long remaining = (LIMIT_MS - (now - lastIntervalStart)) / 100;
  M5.Lcd.setCursor(10, 130);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.printf("Time limit: %2ld.%ld s ", remaining / 10, remaining % 10);

  if (popupVisible && (now - popupShownAt >= POPUP_MS)) {
    M5.Lcd.fillRect(10, 185, 300, 50, BLACK);
    popupVisible = false;
  }
  delay(10);
}
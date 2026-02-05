#include <M5Stack.h>

int current_threshold = 30;
int AButtonSum = 40;
int BButtonSum = 0;

bool flashflag(int ButtonSum) {
  return ButtonSum >= current_threshold;
}

void setup() {
  M5.begin();
  M5.Lcd.setTextSize(3); // 少し大きくしました
  // テキストの基準点を「中央(Middle Center)」に設定
  M5.Lcd.setTextDatum(MC_DATUM); 
}

void loop() {
  // Aボタンの判定
  if (flashflag(AButtonSum)) {
    showCenterText("red", TFT_RED);
    current_threshold += 10; 
  }

  // Bボタンの判定
  if (flashflag(BButtonSum)) {
    showCenterText("blue", TFT_BLUE);
    current_threshold += 10;
  }

  delay(100);
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
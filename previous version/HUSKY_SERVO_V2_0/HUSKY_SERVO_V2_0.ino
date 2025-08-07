#include <HardwareSerial.h>
#include "HUSKYLENS.h"
#include <ESP32Servo.h>

HUSKYLENS huskylens;
Servo panServo;

HardwareSerial huskySerial(2);  // UART2: GPIO 16 (RX), 17 (TX)

const int panPin = 15;           // MG90S 제어 핀
const int centerX = 160;         // 화면 중심 x 좌표
int panAngle = 90;               // 초기 서보 각도

int angle_offset;               // 각도 변화량

void setup() {
  Serial.begin(115200);

  // UART2 초기화
  huskySerial.begin(9600, SERIAL_8N1, 16, 17);
  huskylens.begin(huskySerial);

  // 서보 설정
  panServo.setPeriodHertz(50);
  panServo.attach(panPin, 500, 2500);
  panServo.write(panAngle);

  // HuskyLens 연결 확인 (request만으로 간단히 확인)
  bool connected = false;
  for (int i = 0; i < 5; i++) {
    if (huskylens.request()) {
      connected = true;
      break;
    }
    Serial.println("HuskyLens 연결 시도 중...");
    delay(500);
  }

  if (!connected) {
    Serial.println("❌ HuskyLens 연결 실패");
    while (1);
  } else {
    Serial.println("✅ HuskyLens 연결 성공");
  }

  huskylens.writeAlgorithm(ALGORITHM_TAG_RECOGNITION);  // 태그 인식 모드
}

void loop() {
  if (huskylens.request()) 
  {
    if (huskylens.available()) 
    {
      HUSKYLENSResult result = huskylens.read();
      int x = result.xCenter;

      Serial.print("x: ");
      Serial.println(x);

      // 중심에서의 오차만큼 각도 보정
      int error = x - centerX;
      panAngle -= error * Kp; //error 에 따른 비례 제어
      panAngle = constrain(panAngle, 0, 180);
      panServo.write(panAngle);

      //각도 변화량
      angle_offset = panAngle - 90;
      Serial.print("서보 각도 : "); Serial.print(panAngle);
      Serial.print("도, 변화량 : "); Serial.print(angle_offset); Serial.println("도");
    } 
    else 
    {
      Serial.println("⚠️ 태그 인식되지 않음");
    }
  } 
  else 
  {
    Serial.println("❌ 요청 실패 — 통신 문제");
  }

  delay(100);  // 속도 조절
}

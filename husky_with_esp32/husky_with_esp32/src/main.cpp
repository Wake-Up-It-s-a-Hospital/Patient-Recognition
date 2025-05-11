#include <Arduino.h>
#include <Wire.h>
#include "HUSKYLENS.h"
#include <ESP32Servo.h>

HUSKYLENS huskylens;
Servo panServo;

const int panPin = 14;
const int screenCenterX = 160;  // HuskyLens의 중심 x 좌표
int panAngle = 90;              // 서보 초기 각도

void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);           // SDA = 21, SCL = 22 핀 설정
  huskylens.begin(Wire);

  panServo.setPeriodHertz(50); // MG90S는 50Hz PWM 사용
  panServo.attach(panPin, 500, 2500);  // PWM 범위: 0°=500us, 180°=2500us
  panServo.write(panAngle);     // 초기 중간 위치

  if (!huskylens.knock()) {
    Serial.println("HuskyLens 연결 실패");
    while (1);
  }

  huskylens.writeAlgorithm(ALGORITHM_TAG_RECOGNITION); // 태그 인식 모드 설정
}

void loop() {
  if (huskylens.request()) {
    if (huskylens.available()) {
      HUSKYLENSResult result = huskylens.read();
      int x = result.xCenter;

      Serial.print("x: ");
      Serial.println(x);

      // 간단한 비례 제어(P제어)
      int error = x - screenCenterX;
      panAngle -= error * 0.05;  // 조절 감도 계수

      panAngle = constrain(panAngle, 0, 180);
      panServo.write(panAngle);
    }
  }

  delay(50);  // 제어 간격 (20~50ms 추천)
}

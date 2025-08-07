#include <Wire.h>
//#define HUSKYLENS_SLAVE_I2C
#include "HUSKYLENS.h"
#include <ESP32Servo.h>
#include <HardwareSerial.h>

HUSKYLENS huskylens;
Servo panServo;

// STM32와 연결된 UART2 (GPIO 16 RX, GPIO 17 TX)
HardwareSerial stm32Serial(2);

const int panPin = 15;           // MG90S 제어 핀
const int centerX = 160;         // 화면 중심 x 좌표
int panAngle = 90;               // 초기 각도
int angle_offset = 0;           // 서보모터 각 변화량
int tx_buf = 0;                 // 왼쪽 -90 <<<<<<<>>>>>>>>  +90 오른쪽 으로 각도 변경을 위한 변환  tx_buf = -(angle_offset);

//문자열 
char str_int[4];
char tx_int[5];

const float Kp = 0.05;           // 비례 제어 계수

void setup() {
  Serial.begin(115200);                   // PC용 시리얼
  Wire.begin(21, 22);                     // I2C SDA, SCL
  huskylens.begin(Wire);                 // HuskyLens I2C 초기화

  stm32Serial.begin(9600, SERIAL_8N1, 16, 17); // STM32와 UART2 통신 시작

  panServo.setPeriodHertz(50);
  panServo.attach(panPin, 500, 2500);
  panServo.write(panAngle);

  // HuskyLens 연결 확인
  bool connected = false;
  for (int i = 0; i < 5; i++) 
  {
    if (huskylens.request()) 
    {
      connected = true;
      break;
    }
    Serial.println("HuskyLens 연결 시도 중...");
    delay(500);
  }

  if (!connected) 
  {
    Serial.println("❌ HuskyLens 연결 실패");
    while (1);
  } 
  else 
  {
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

      // 서보 제어
      int error = x - centerX;
      panAngle -= error * Kp;
      //panAngle = constrain(panAngle, 0, 180);
      panAngle = constrain(panAngle, 20,160); //허스키렌즈 각도 -70<angle_offset<70 으로 제한 
      panServo.write(panAngle);

      // 변화량 계산 및 전송
      angle_offset = panAngle - 90;

      /*if((angle_offset >= 70) && (angle_offset <=-70)) //angle_offset  abs(angle_offset)넘을시 70으로 고정 abs(angle limit) = 70
      {
        if(angle_offset <0)
        {
          angle_offset = -70;
        }
        else
        {
          angle_offset = 70;
        }
      }
      */

      tx_buf = -(angle_offset);

      // ESP32 시리얼 디버깅 출력
      Serial.print("서보 각도: ");
      Serial.print(panAngle);
      Serial.print("도, 변화량: ");
      Serial.print(tx_buf);
      Serial.println("도");

      /*
      stm32Serial.print(tx_buf);
      stm32Serial.print('/');
      // HardwareSerial - 
      */


      sprintf(str_int, "%03d", tx_buf);
      sprintf(tx_int, "%s/", str_int);

      stm32Serial.print(tx_int);
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

  delay(100);
}

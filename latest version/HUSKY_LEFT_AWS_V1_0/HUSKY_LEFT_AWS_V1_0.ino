#include <Wire.h>
//#define HUSKYLENS_SLAVE_I2C
#include "HUSKYLENS.h"
#include <HardwareSerial.h>

#include <Arduino.h>                    
#include <IO7F32.h>                    

String user_html = "";                  
char* ssid_pfix = (char*)"Husky";      
unsigned long lastPublishMillis = -pubInterval; 


#define BUTTON_PIN 15             // 추후 stm의 버튼으로 변경 
#define LED_PIN 19                // 추후 stm의 led로 변경

bool currentMode = false;               // 디폴트 manual 모드   
String currentModeText = "manual";       

// 상태 감지 변수들
bool lost = false;                       // lost 상태 변수
bool battery = true;                       
bool lastLostState = false;              // 이전 lost 상태 (변화 감지용)


int lastButtonState = HIGH;       
unsigned long lastDebounceTime = 0; 
const unsigned long debounceDelay = 200; 

volatile bool modeChanged = false; 


void IRAM_ATTR buttonInterrupt() {
  modeChanged = true;  
}


// Lost 알림을 AWS IoT에 publish하는 함수
void publishLostAlert() {
  StaticJsonDocument<256> doc;                         
  JsonObject data = doc.createNestedObject("d");     

  data["alert_type"] = "lost";                      
  data["device_id"] = WiFi.macAddress();              
  data["device_type"] = "husky_alert";              
  data["source_device"] = WiFi.macAddress();          
  data["timestamp"] = millis();                       
  data["message"] = "Husky device lost detected";     

  serializeJson(doc, msgBuffer);                        
  
 
  strcpy(evtTopic, "iot3/Husky/evt/alert/fmt/json");  

  if (client.publish(evtTopic, msgBuffer)) {            
    Serial.println(" Lost alert published to AWS IoT:");  
    Serial.println(msgBuffer);
  } else {
    Serial.println(" Failed to publish lost alert");
  }
}

// Battery 부족 알림을 AWS IoT에 publish하는 함수
void publishBatteryAlert() {
  StaticJsonDocument<256> doc;                        
  JsonObject data = doc.createNestedObject("d");     

  data["alert_type"] = "battery";                    
  data["device_id"] = WiFi.macAddress();            
  data["device_type"] = "husky_alert";                
  data["source_device"] = WiFi.macAddress();        
  data["battery_level"] = battery;                  
  data["timestamp"] = millis();                     
  data["message"] = "Low battery detected";          

  serializeJson(doc, msgBuffer);                        
  
  
  strcpy(evtTopic, "iot3/Husky/evt/alert/fmt/json");  

  if (client.publish(evtTopic, msgBuffer)) {           
    Serial.println(" Battery alert published to AWS IoT:");  
    Serial.println(msgBuffer);
  } else {
    Serial.println(" Failed to publish battery alert");
  }
}


void publishMode() {
  StaticJsonDocument<256> doc;                       
  JsonObject data = doc.createNestedObject("d");     

  data["mode_sync"] = currentModeText;                
  data["device_id"] = WiFi.macAddress();              
  data["device_type"] = "husky_controller";         
  data["source_device"] = WiFi.macAddress();        
  data["timestamp"] = millis();                       
  data["command"] = "sync_mode";                    

  serializeJson(doc, msgBuffer);                        
  
 
  strcpy(evtTopic, "iot3/Husky/evt/mode/fmt/json");  

  if (client.publish(evtTopic, msgBuffer)) {          
    Serial.println(" Mode sync published to AWS IoT:");  
    Serial.println(msgBuffer);
  } else {
    Serial.println(" Failed to publish mode sync");
  }
}


void handleUserCommand(char* topic, JsonDocument* root) {
    JsonObject d = (*root)["d"];        
    
    Serial.println("\n=== 명령 수신 ===");        
    Serial.println("토픽: " + String(topic));     
    
    
    String msgStr;
    serializeJson(*root, msgStr);         
    Serial.println("수신 메시지: " + msgStr); 
    
    
    if (d.containsKey("mode_sync") && d.containsKey("device_type")) {
        String deviceType = d["device_type"];  
     
        if (deviceType == "watch_controller") {
            String newMode = d["mode_sync"];  
            String sourceDevice = d["source_device"]; 
            
            Serial.println("  시계부 컨트롤러에서 모드 동기화 명령 수신");
            Serial.println("디바이스 타입: " + deviceType);   
            Serial.println("새로운 모드: " + newMode);      
            Serial.println("시계부 디바이스: " + sourceDevice); 
           

            String oldMode = currentModeText;           
            currentModeText = newMode;                    
            currentMode = (newMode == "auto");          
            
            // LED 상태 업데이트
            digitalWrite(LED_PIN, currentMode ? HIGH : LOW);
            
            Serial.println("  모드 동기화 완료");
            Serial.println("   이전 모드: " + oldMode); 
            Serial.println("   현재 모드: " + currentModeText); // 현재 모드 출력
            Serial.println("   currentMode: " + String(currentMode ? "true" : "false")); // bool 상태 출력
            
        } else {
            Serial.println("  다른 디바이스에서 온 명령: " + deviceType); // watch_controller가 아닐 경우
        }
    } else {
        Serial.println("  mode_sync 또는 device_type 필드 없음"); // 필수 키 누락 시
    }
    
    Serial.println("==================\n");    // 처리 완료 구분선
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//허스리 관련 파라미터
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
HUSKYLENS huskylens;

// STM32와 연결된 UART2 (GPIO 16 RX, GPIO 17 TX)
HardwareSerial stm32Serial(2);


//허스키 관련 변수
const int centerX = 160;         // 화면 중심 x 좌표
int tx_buf = 0;                 // 왼쪽 -90 <<<<<<<>>>>>>>>  +90 오른쪽 으로 각도 변경을 위한 변환  tx_buf = -(angle_offset);
unsigned long previousMillis = 0; // millis 함수 저장 변수

//통신 문자열
char str_int[4]; 
char tx_int[5];

// 통신 관련 변수 (RX)
char uartRxBuffer[32];      // STM32 -> ESP32    UART 수신 버퍼
char batteryValue[2];       // 문자열 "0" - 배터리 부족 , "1" - 배터리 상태 괜찮음
char lostValue[2];          // 문자열 "0" - 비발생 , "1" - 발생

unsigned int rxIndex = 0;   // 수신 인덱스
bool dataComplete = false;  // 수신 완료 플래그

// 3. STM32에서 데이터를 받는 함수
// 3. STM32에서 데이터를 받는 함수 (수정된 버전)
void receiveFromSTM32() {
  // STM32에서 데이터가 온 경우
  if (stm32Serial.available()) {
    char receivedByte = stm32Serial.read();  // 한 바이트 읽기
    
    // 받은 값이 '0' 또는 '1'인지 확인
    if (receivedByte == '0' || receivedByte == '1') {
      lostValue[0] = receivedByte;  // 받은 값 저장
      lostValue[1] = '\0';          // null 종료 문자
      
      // 디버깅 출력
      Serial.print("STM32에서 수신: ");
      Serial.println(lostValue);
      
      // 값에 따른 처리
      if (lostValue[0] == '1') {
        Serial.println(" Lost 감지됨");
        lost = true;
      } else {
        Serial.println(" 정상 상태");
        lost = false;
      }
    }
    else {
      // 잘못된 값이 온 경우
      Serial.print("잘못된 값 수신: ");
      Serial.println(receivedByte);
    }
  }
}


void setup() 
{ 
  Serial.begin(115200);                   // PC용 시리얼
  Wire.begin(21, 22);                     // I2C SDA, SCL
  huskylens.begin(Wire);                 // HuskyLens I2C 초기화

  stm32Serial.begin(9600, SERIAL_8N1, 16, 17); // STM32와 UART2 통신 시작
      // aws 관련 파트 중간 삽입 (버튼 및 LED 핀 설정)
      pinMode(BUTTON_PIN, INPUT_PULLUP);                   
      pinMode(LED_PIN, OUTPUT);                           
      attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonInterrupt, FALLING); 
      
      Serial.println("\n 허스키부 ESP32 시작 (Watchdog 안전 버전)"); 
      Serial.println("목표: currentModeText 변수 양방향 동기화 + Lost/Battery 알림");
      Serial.println("Device ID: " + String(ssid_pfix)); 
      // 초기 상태 출력
      Serial.println("\n초기 상태:");
      Serial.println("  currentModeText = " + currentModeText); 
      Serial.println("  currentMode = " + String(currentMode ? "true" : "false")); 
      Serial.println("  lost = " + String(lost ? "true" : "false")); 
     
     // lostValue 초기화
      lostValue[0] = '0';  // 기본값 '0'
      lostValue[1] = '\0'; // null 종료 문자
  // IO7F32 라이브러리 초기화 및 설정 불러오기
    initDevice();                        
    JsonObject meta = cfg["meta"];    
    pubInterval = meta.containsKey("pubInterval") ? meta["pubInterval"] : 0; 
    lastPublishMillis = -pubInterval;    
    
    // WiFi 연결 설정 (Watchdog 안전)
    WiFi.mode(WIFI_STA);                 
    WiFi.begin((const char*)cfg["ssid"], (const char*)cfg["w_pw"]); 
    
    Serial.print("\nWiFi 연결 중");
    int wifiAttempts = 0;
    while (WiFi.status() != WL_CONNECTED && wifiAttempts < 60) { // 
        delay(500);                      
        Serial.print(".");            
        wifiAttempts++;
        yield(); // Watchdog 리셋
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n WiFi 연결 완료"); 
        Serial.println("IP: " + WiFi.localIP().toString());   
        Serial.println("MAC: " + WiFi.macAddress());           // MAC 주소 출력
    } else {
        Serial.println("\n WiFi 연결 실패 - 계속 진행");
    }
    
    // AWS IoT 연결 설정
    userCommand = handleUserCommand;      
    set_iot_server();                 
    iot_connect();                      
    
    digitalWrite(LED_PIN, LOW);
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
    Serial.println("HuskyLens 연결 실패");
   // while (1);
  } 
  else 
  {
    Serial.println("HuskyLens 연결 성공");
  }

  huskylens.writeAlgorithm(ALGORITHM_TAG_RECOGNITION);  // 태그 인식 모드 진입

    Serial.println("\n 허스키부 준비 완료"); // 준비 완료 로그
    Serial.println(" 시계부와 양방향 동기화 준비 완료"); // 양방향 동기화 안내
    Serial.println(" Lost 감지 시 시계부 진동 알림 전송");
    Serial.println(" Watchdog 패닉 방지 기능 활성화");
    Serial.println(" 똑따라링 화이팅");
    Serial.println("======================================="); // 구분선
}

void loop() 
{
  if (!client.connected()) {           // 연결이 끊어졌는지 확인
        Serial.println(" 재연결 시도...");
        iot_connect();                   // 재연결 시도
        delay(1000);                     // 재연결 대기 (2초 → 1초로 단축)
    }
    client.loop();                       // MQTT 메시지 처리
    
    // Watchdog 리셋 (패닉 방지)
    yield();

  //////////////////////////// //////////////////////////////
  ///////////   허스키렌즈 x 값 받아오는 Logic //////////////
  ///////////////////////////////////////////////////////////
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= 100)  // 100ms 주기
  {
    previousMillis = currentMillis;

    if (huskylens.request()) //허스키렌즈 값 요청
    {
        bool id1Found = false;
        int x = 0;

        while (huskylens.available()) 
        {
            HUSKYLENSResult result = huskylens.read();

            if (result.ID == 1)  // ID 1만 처리
            {
                x = result.xCenter;
                id1Found = true;
                break;  // ID 1 찾았으니 종료
            }
        }

        if (id1Found)
        {   int leftAngle = ((x * 70) + 160) / 320 - 70;
            tx_buf = leftAngle;

            sprintf(str_int, "%03d", tx_buf); // 문자열로 변환 03d 형태 
            sprintf(tx_int, "%s/", str_int); // 구분자 추가

            stm32Serial.print(tx_int); // 구분자까지 추가한 버퍼 stm32로 uart tx이용하여 송신
            // ===== + currentMode 전송 =====
            // currentMode 
            stm32Serial.print(currentMode);  // 예: "1"
            stm32Serial.print(".");          // 예: "."


           /* Serial.print("ID 1 x: "); 
            Serial.print(x);
            Serial.print(" currentMode 전송: ");
            Serial.println(currentMode); */
           
        }
        else
        {
            stm32Serial.print("999/");
            stm32Serial.print(currentMode);
            stm32Serial.print(".");
           /* Serial.print("ID 1 태그 인식되지 않음");
            Serial.print(" currentMode 전송: ");
            Serial.println(currentMode); */
        }
    }
    else 
    {
        Serial.println("요청 실패 — 통신 문제");
    }
  }
  //////////////////////////// //////////////////////////////
  ///////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////
  ///////////// STM32 -> ESP32 UART RX 통신 로직 /////////////
  ///////////////////////////////////////////////////////////
   // UART 수신 (문자 단위, 예: "1/0/" 형태)
   // UART 수신 처리


   receiveFromSTM32();
   



 


  

  ///////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////

  // 다른 비동기 처리 구문
  // 버튼 디바운스 + 모드 토글 처리
    int currentState = digitalRead(BUTTON_PIN);
    if (currentState == LOW && lastButtonState == HIGH) {
        unsigned long now = millis();                       
        if (now - lastDebounceTime > debounceDelay) {
            currentMode = !currentMode;                        // 모드 토글
            currentModeText = currentMode ? "auto" : "manual";
            modeChanged = true;                              
            lastDebounceTime = now;                          
        }
    }
    lastButtonState = currentState;

    // 모드 변경 시 publish 및 LED 업데이트
    if (modeChanged) {
        Serial.println(" Mode toggled → " + currentModeText);
        publishMode();                                     // 시계부로 동기화 메시지 전송
        modeChanged = false;                              
        digitalWrite(LED_PIN, currentMode ? HIGH : LOW);    // LED 상태 업데이트
    }
    
    // Watchdog 리셋 (패닉 방지)
    yield();
    
 


   


  
    // Lost 상태 변화 감지
    
    if(lost == true){
      Serial.println(" LOST 감지 시계부로 알림 전송");
      publishLostAlert();
      yield(); // publish 후 Watchdog 리셋
      lost =false;
    }
    
  
   
    if(battery == false  ){
      Serial.println(" 배터리 부족 감지하여 시계부로 알림 전송함 ");
      publishBatteryAlert();
      battery = true;
      yield(); // publish 후 Watchdog 리셋
      
    }

   
    // 루프 끝에서 Watchdog 리셋
    //delay(50);                           // CPU 부하 감소 (20ms → 50ms로 증가)
    yield();             
    
    
    
    
                    // Watchdog 리셋
}

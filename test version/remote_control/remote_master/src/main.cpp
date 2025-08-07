#include <SoftwareSerial.h>
#include <Arduino.h>

#define x = A0;
#define y = A1;

SoftwareSerial HC05(2, 3);

int x_map, y_map;
char x_str[4];
char y_str[4];
char data[9];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  HC05.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  x_map = analogRead(A0);
  x_map = map(x_map, 0, 1023, 0, 1000);
  y_map = analogRead(A1);
  y_map = map(y_map, 0, 1023, 0, 1000);
  sprintf(x_str, "%03d", x_map / 10);
  sprintf(y_str, "%03d", y_map / 10);
  sprintf(data, "%s,%s.", x_str, y_str);
  Serial.println(data);
  HC05.print(data);
  
  delay(200);
}

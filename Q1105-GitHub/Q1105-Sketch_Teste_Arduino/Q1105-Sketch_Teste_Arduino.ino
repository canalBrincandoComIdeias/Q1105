#include <SoftwareSerial.h>

SoftwareSerial moduloWiFi(8, 9); // RX, TX

void setup() {
  Serial.begin(9600);
  moduloWiFi.begin(9600);

  Serial.println("Arduino iniciado!");
}

void loop() {
  while (moduloWiFi.available()) {
    Serial.write(moduloWiFi.read());
  }
  while (Serial.available()) {
    moduloWiFi.write(Serial.read());
  }
}

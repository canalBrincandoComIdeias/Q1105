#include <SoftwareSerial.h>
#include "LedController.hpp"

#define pinLED 13
#define pinRX  8
#define pinTX  9

#define pinDIN 10
#define pinCS  11
#define pinCLK 12

unsigned long connectTimeout = 0;      //em milisegundos (zero para tentar conexão sem timeout)
unsigned long retryInterval = 10000;   //em milisegundos

SoftwareSerial moduloWiFi(pinRX, pinTX);  // RX, TX
LedController<1, 1> display;

float fTemperatura;
float fTemperaturaAnt = 0;
float fUmidade;
float fUmidadeAnt = 0;

String pacote, comando, topico, valor;

void setup() {
  pinMode(pinLED, OUTPUT);
  digitalWrite(pinLED, HIGH);

  display = LedController<1, 1>(pinDIN, pinCLK, pinCS);

  display.init(pinCS);
  display.setIntensity(15);
  display.clearMatrix();

  Serial.begin(9600);
  moduloWiFi.begin(9600);

  subscribe("temp");
  subscribe("umid");

  display.setLed(0, 7, 0, HIGH);
  display.setLed(0, 6, 0, HIGH);
  display.setLed(0, 5, 0, HIGH);
  display.setLed(0, 3, 0, HIGH);
  display.setLed(0, 2, 0, HIGH);
  display.setLed(0, 1, 0, HIGH);
  display.setLed(0, 0, 0, HIGH);

  digitalWrite(pinLED, LOW);
  Serial.println("Sketch iniciado!");

  moduloWiFi.print("pub:req:1;");
}

void loop() {
  if (moduloWiFi.available()) {
    pacote = moduloWiFi.readStringUntil(';');
    pacote.trim();

    int sep1 = pacote.indexOf(':');
    if (sep1 != -1) {
      topico = pacote.substring(0, sep1);
      comando.trim();

      valor = pacote.substring(sep1 + 1);
      topico.trim();

      if (topico == "temp") {
        fTemperatura = valor.toFloat();

        Serial.print("temp:");
        Serial.print(fTemperatura, 2);
        Serial.println(";");
        mostraTemp();
      }

      if (topico == "umid") {
        fUmidade = valor.toFloat();

        Serial.print("umid:");
        Serial.print(fUmidade, 2);
        Serial.println(";");
        mostraUmid();
      }
    }
  }

  fTemperaturaAnt = fTemperatura;
  fUmidadeAnt = fUmidade;
}

void mostraTemp() {
  int iTemp = fTemperatura;
  int iDec  = int(round(fTemperatura * 100)) % 100;

  display.setDigit(0, 3, (((iTemp % 1000) % 100) / 10), false);
  display.setDigit(0, 2, (((iTemp % 1000) % 100)) % 10, true);

  display.setDigit(0, 1, iDec / 10, false);
  //display.setDigit(0, 0, iDec % 10, false);

  display.setLed(0, 0, 0, HIGH);
  display.setLed(0, 0, 1, HIGH);
  display.setLed(0, 0, 5, HIGH);
  display.setLed(0, 0, 6, HIGH);
}

void mostraUmid() {
  int iUmid = fUmidade;
  int iDec  = int(round(fUmidade * 100)) % 100;

  display.setDigit(0, 7, (((iUmid % 1000) % 100) / 10), false);
  display.setDigit(0, 6, (((iUmid % 1000) % 100)) % 10, true);
  display.setDigit(0, 5, iDec / 10, false);
}

bool subscribe(String topico) {
  unsigned long timeOut;
  unsigned long timeRetry;
  String pacote;
  bool confirmado = false;

  moduloWiFi.println("sub:" + topico + ";");
  timeOut = millis();
  timeRetry = millis();

  Serial.println("sub:" + topico + ";");

  while (((millis() - timeOut) > connectTimeout) || (connectTimeout == 0)) {
    if (moduloWiFi.available()) {
      pacote = moduloWiFi.readStringUntil(';');
      pacote.trim();

      Serial.println(pacote);

      if (pacote == ("->sub:" + topico)) {
        return true;
      }
    }

    //Nova tentativa, enquanto não tem resposta de confirmação
    if ((millis() - timeRetry) > retryInterval) {
      moduloWiFi.println("sub:" + topico + ";");
      timeRetry = millis();
    }
  }

  return false;
}

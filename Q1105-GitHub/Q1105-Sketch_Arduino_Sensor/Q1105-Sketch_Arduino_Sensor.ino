#include <SoftwareSerial.h>
#include <DHT.h>

#define pinSensor 2
#define pinLED 13

#define DHTTYPE DHT22

SoftwareSerial moduloWiFi(10, 11);  // RX, TX
DHT dht(pinSensor, DHTTYPE);

unsigned long connectTimeout = 0;     //em milisegundos (zero para tentar conexão sem timeout)
unsigned long retryInterval = 10000;  //em milisegundos

float fTemperatura;
float fTemperaturaAnt = 0;
float fUmidade;
float fUmidadeAnt = 0;

String pacote, topico, comando;
bool pedido = false;

void setup() {
  pinMode(pinLED, OUTPUT);
  digitalWrite(pinLED, HIGH);

  Serial.begin(9600);
  moduloWiFi.begin(9600);

  dht.begin();
  subscribe("req");

  digitalWrite(pinLED, LOW);
  Serial.println("Sketch iniciado!");
}

void loop() {
  fTemperatura = dht.readTemperature();
  fUmidade = dht.readHumidity();

  if ((fTemperatura != fTemperaturaAnt) || (pedido)) {
    moduloWiFi.print("pub:temp:");
    moduloWiFi.print(fTemperatura);
    moduloWiFi.println(";");

    Serial.print("pub:temp:");
    Serial.print(fTemperatura);
    Serial.println(";");
  }

  if ((fUmidade != fUmidadeAnt) || (pedido)) {
    moduloWiFi.print("pub:umid:");
    moduloWiFi.print(fUmidade);
    moduloWiFi.println(";");

    Serial.print("pub:umid:");
    Serial.print(fUmidade);
    Serial.println(";");

    pedido = false;
  }

  if (moduloWiFi.available()) {
    pacote = moduloWiFi.readStringUntil(';');
    pacote.trim();

    Serial.println(pacote);

    int sep1 = pacote.indexOf(':');
    if (sep1 != -1) {
      topico = pacote.substring(0, sep1);
      comando.trim();

      if (topico == "req") {
          pedido = true;
      }
    }
  }

  fTemperaturaAnt = fTemperatura;
  fUmidadeAnt = fUmidade;
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

#include <WiFi.h>
#include <MQTT.h>
#include "user.h"  //Informar a rede WiFi, a senha, usuário MQTT e a senha

//const char ssid[] = "xxx";    
//const char pass[] = "xxx";

//Servidor MQTT
const char servidor[] = "54.233.221.233";
//const char servidor[] = "mqtt.monttudo.com";  //(caso o IP não funcione, use a identificação pelo domínio)

//Usuário MQTT
const char deviceName[] = "ESP32C3-2";
const char mqttUser[] = "todos";
const char mqttPass[] = "cortesi@BrincandoComIdeias";

#define pinLEDPlaca 8

WiFiClient net;
MQTTClient client;

String pacote, comando, topico, valor;
unsigned long lastMillis = 0;

//Comandos a serem recebidos via serial
//sub:<tópico>;         //subscribe
//uns:<tópico>;         //unsubscribe
//pub:<tópico>:<valor>; //publish

//Enviado via serial quando pacote for recebido
//<tópico>:<valor>;

void connect() {
  Serial1.print("->wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial1.print(".");
    delay(1000);
  }
  Serial1.println(";");

  Serial1.print("->mqtt...");
  while (!client.connect(deviceName, mqttUser, mqttPass)) {
    Serial1.print(".");
    delay(1000);
  }
  Serial1.println(";");
  Serial1.println("->conectado;");
}

void messageReceived(String &topic, String &payload) {
  Serial1.println(topic + ":" + payload);

  // Nota: Não use o "client" nessa função para publicar, subscrever ou
  // dessubscrever porque pode causar problemas quando outras mensagens são recebidas
  // enquanto são feitos envios ou recebimentos. Então, altere uma variavel global,
  // coloque numa fila e trate essa fila no loop depois de chamar `client.loop()`.
}

void setup() {
  pinMode(pinLEDPlaca, OUTPUT);
  digitalWrite(pinLEDPlaca, LOW);

  Serial1.begin(9600, SERIAL_8N1, 20, 21);   //VELOCIDADE, CONFIGURACAO_SERIAL, RX, TX
  WiFi.begin(ssid, pass);

  // Observação: Nomes de domínio locais (por exemplo, "Computer.local" no OSX)
  // não são suportados pelo Arduino. Você precisa definir o endereço IP diretamente.
  client.begin(servidor, net);
  client.onMessage(messageReceived);

  connect();
  digitalWrite(pinLEDPlaca, HIGH);
}

void loop() {
  client.loop();
  delay(10);  //corrige alguns problemas com a estabilidade do WiFi

  if (!client.connected()) {
    connect();
  }

  if (Serial1.available()) {
    pacote = Serial1.readStringUntil(';');
    pacote.trim();

    int sep1 = pacote.indexOf(':');
    if (sep1 != -1) {
      comando = pacote.substring(0, sep1);
      comando.trim();

      topico = pacote.substring(sep1 + 1);
      topico.trim();

      if (comando == "sub") {
        client.subscribe(topico);
        Serial1.println("->sub:" + topico + ";");
      }

      if (comando == "uns") {
        client.unsubscribe(topico);
        Serial1.println("->uns:" + topico + ";");
      }

      if (comando == "pub") {
        int sep2 = topico.indexOf(':');
        if (sep2 != -1) {
          valor = topico.substring(sep2 + 1);
          valor.trim();

          topico = topico.substring(0, sep2);
          topico.trim();

          client.publish(topico, valor);
          Serial1.println("->pub:" + topico + ":" + valor + ";");
        }
      }
    }
  }
}

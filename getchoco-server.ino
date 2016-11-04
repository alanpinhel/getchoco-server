#include <SPI.h>
#include <Ethernet.h>
#include "Servo.h"

typedef struct {
  int mini;  // posição mínima de grau do motor
  int maxi;  // posição máxima de grau do motor
  Servo m;   // motor
} motor;

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED // MAC genérico
};

EthernetServer servidor(80); // Servidor ouvindo na porta 80

String httpGet = ""; // Requisição GET

boolean comecaLer = false; // Flag para iníciar leitura dos parâmetros GET

motor motorBase, motorBraco, motorMao; // Motores

/**
   Inicializa Serial e WebServer.
*/
void setup() {
  Serial.begin(9600);
  Ethernet.begin(mac);
  servidor.begin();

  carregaConfiguracoesMotor();
  Serial.println(Ethernet.localIP());
}

/**
   Carrega as configurações iniciais para os motores.
*/
void carregaConfiguracoesMotor() {
  motorBase.mini = 70;
  motorBase.maxi = 150;
  motorBase.m.attach(5);

  motorBraco.mini = 122;
  motorBraco.maxi = 170;
  motorBraco.m.attach(10);

  motorMao.mini = 70;
  motorMao.maxi = 90;
  motorMao.m.attach(11);

  moveMotor(motorMao, motorMao.maxi);
  moveMotor(motorBase, motorBase.maxi);
  moveMotor(motorBraco, motorBraco.maxi);
}

/**
   Responde requisição HTTP com status 200 OK, libera acesso CORS a
   qualquer cliente externo e finaliza conexão.
*/
void loop() {
  EthernetClient cliente = servidor.available();
  if (cliente) {
    cliente.println("HTTP/1.1 200 OK");
    cliente.println("Access-Control-Allow-Origin: *");
    cliente.println("Connection: close");

    processaCliente(cliente);

    String motor = pegaMotor();
    int angulo = pegaAngulo();

    acionaMotor(motor, angulo);
    Serial.println(motor + " -> " + angulo);

    httpGet = "";
  }
}

/**
   Processa requisição do cliente.
   Parâmetros:
      cliente: cliente que efetuou a requisição.
*/
void processaCliente(EthernetClient cliente) {
  boolean linhaEmBranco = true;

  while (cliente.connected()) {
    if (cliente.available()) {
      char c = cliente.read();

      if (comecaLer && c == ' ') {
        comecaLer = false;
      }
      if (c == '?') {
        comecaLer = true;
      }
      if (comecaLer && c != '?') {
        httpGet += c;
      }
      if (c == '\n' && linhaEmBranco) {
        break;
      }
      if (c == '\n') {
        linhaEmBranco = true;
      }
      else if (c != '\r') {
        linhaEmBranco = false;
      }
    }
  }

  delay(1);
  cliente.stop();
}

/**
   Extrai da requisição GET somente o nome do motor.
   Retorna:
      nome do motor
*/
String pegaMotor() {
  int inicio = httpGet.indexOf("=") + 1;
  int fim = httpGet.indexOf("&");
  return httpGet.substring(inicio, fim);
}

/**
   Extrai da requisição GET somente o valor do ângulo.
   Retorna:
      valor do ângulo
*/
int pegaAngulo() {
  int inicio = httpGet.lastIndexOf("=") + 1;
  int fim = httpGet.length();
  return httpGet.substring(inicio, fim).toInt();
}

/**
   Aciona o motor.
   Parâmetros:
      nomeMotor: nome do motor a ser movido
      anguloMotor: ângulo do motor para qual será movido.
*/
void acionaMotor(String nomeMotor, int anguloMotor) {
  if (nomeMotor.equals("base"))
    moveMotor(motorBase, anguloMotor);
  else if (nomeMotor.equals("braco"))
    moveMotor(motorBraco, anguloMotor);
  else if (nomeMotor.equals("mao"))
    moveMotor(motorMao, anguloMotor);
}

/**
   Move o motor para o respectivo ângulo.
   Parâmetros:
      mot: motor que será movido
      grau: grau para que o motor será movido.
*/
void moveMotor(motor mot, int grau) {
  if (grau >= mot.mini && grau <= mot.maxi)
    mot.m.write(grau);
}

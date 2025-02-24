
#include "esp32-hal-gpio.h"
#include "esp32-hal.h"
#ifndef ROBOT_H
#define ROBOT_H

#include <WiFi.h>
#include <ESP32Servo.h>


#define EchoPin 5
#define TrigPin 18
#define ENA 4
#define IN1 15
#define IN2 2
#define ENB 21
#define IN3 22
#define IN4 23
#define LED1 27
#define LED2 26
#define LED3 25

class Robot {
public:
  int lim = 50;
  const char *ssid;
  const char *password;
  WiFiServer *server;
  bool motor_activo = false;

  Robot(const char *wifiSSID, const char *wifiPassword, WiFiServer *wifiServer) {
    ssid = wifiSSID;
    password = wifiPassword;
    server = wifiServer;

    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);
    pinMode(EchoPin, INPUT);
    pinMode(TrigPin, OUTPUT);
    pinMode(ENA, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(ENB, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    Serial.begin(115200);
  }

  void conectar_WiFi() {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1500);
      Serial.print(".");
    }
    Serial.println("\nWiFi conectado. IP:");
    Serial.println(WiFi.localIP());
    server->begin();  // Usamos el puntero para iniciar el servidor
  }

  void manejar_cliente(WiFiClient &client) {
    Serial.println("Nuevo Cliente");
    String request = "";

    while (client.connected()) {
      if (client.available()) {
        String line = client.readStringUntil('\n');
        request += line;
        if (line == "\r") { break; }
      }
    }

    // Serial.println("Request: " + request);

    char comando = request.charAt(5);
    String estado = "";

    switch (comando) {
      case 'W':
        avanzar();
        estado = "{\"estado\": \"Avanzando\"}";
        break;
      case 'A':
        girar_izquierda();
        estado = "{\"estado\": \"Girando a la izquierda\"}";
        break;
      case 'S':
        parar();
        estado = "{\"estado\": \"Parado\"}";
        break;
      case 'D':
        girar_derecha();
        estado = "{\"estado\": \"Girando a la derecha\"}";
        break;
      default: estado = "{\"estado\": \"Comando no reconocido\"}"; break;
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Connection: close");
    client.println();
    client.println(estado);
    client.stop();
  }

  // Métodos de movimiento...
  void avanzar() {
    if (medir_distancia()) {
      motor_activo = true;
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      analogWrite(ENA, 255);
      // Serial.println("Avanzando...");
      analogWrite(LED1, 80);
    }
  }

  void parar() {
    motor_activo = false;
    analogWrite(LED1, 0);
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 255);
    delay(400);
    analogWrite(ENA, 0);
    // Serial.println("Parado.");
  }

  void girar_izquierda() {
    analogWrite(LED2, 80);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENB, 110);
    delay(1000);
    analogWrite(ENB, 0);
    // Serial.println("Girando a la izquierda...");
    analogWrite(LED2, 0);
  }

  void girar_derecha() {
    analogWrite(LED3, 80);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, 110);
    delay(1000);
    analogWrite(ENB, 0);
    // Serial.println("Girando a la derecha...");
    analogWrite(LED3, 0);
  }

  bool medir_distancia() {
    int muestras = 5;
    float valores[muestras];
    bool seguro;

    for (int i = 0; i < muestras; i++) {
      digitalWrite(TrigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(TrigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(TrigPin, LOW);
      long duration = pulseIn(EchoPin, HIGH, 30000);
      valores[i] = duration / 58.2;
      delay(10);
    }

    // Ordenar los valores para obtener la mediana
    for (int i = 0; i < muestras - 1; i++) {
      for (int j = i + 1; j < muestras; j++) {
        if (valores[i] > valores[j]) {
          float temp = valores[i];
          valores[i] = valores[j];
          valores[j] = temp;
        }
      }
    }
    seguro = valores[muestras / 2] > lim;


    if (!seguro && motor_activo) {
      Serial.println("\nPase de seguridad!!!\n");
      parar();
    }
    return seguro;
  }
};

#endif

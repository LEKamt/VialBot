#include "Robot.h"

const char *ssid = "";
const char *password = "";
WiFiServer server(80);

Robot VB(ssid, password, &server);

void setup() {
  VB.conectar_WiFi();
}

void loop() {
  bool n = VB.medir_distancia();
  WiFiClient client = server.available();
  if (client) {
    VB.manejar_cliente(client);
  }
}

#include <WiFi.h>
#include <WebServer.h>
#include "DHT.h"

// Defina suas credenciais WiFi
const char* ssid = "Rede WiFi";
const char* password = "Senha WiFi";

// Define o pino e tipo do sensor
#define DHTPIN 15         // Substitua pelo GPIO usado para o DATA
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

void handleRoot() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    server.send(500, "text/plain", "Falha ao ler do sensor DHT!");
    return;
  }

  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<title>Clima Local</title></head><body>";
  html += "<h1>Dados do Sensor DHT22</h1>";
  html += "<p>Temperatura: " + String(t) + " &deg;C</p>";
  html += "<p>Umidade: " + String(h) + " %</p>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void loop() {
  server.handleClient();
}
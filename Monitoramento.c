#include <WiFi.h>
#include <WebServer.h>
#include "DHT.h"

// Defina suas credenciais WiFi
const char* ssid = "Rede WiFi";
const char* password = "Senha WiFi";

// Define o pino e tipo do sensor
#define DHTPIN 4       // Substitua pelo GPIO usado para o DATA
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

  String html = R"rawliteral(
    <!DOCTYPE html>
    <html lang="pt-BR">
    <head>
      <meta charset="UTF-8" />
      <meta name="viewport" content="width=device-width, initial-scale=1.0" />
      <title>Monitoramento Ambiente - ESP32</title>
      <style>
        body {
          font-family: Arial, sans-serif;
          text-align: center;
          background: linear-gradient(to right, #00c6ff, #0072ff);
          color: white;
          padding: 50px;
        }
        .container {
          background: rgba(255, 255, 255, 0.1);
          border-radius: 10px;
          padding: 30px;
          display: inline-block;
          margin-top: 50px;
        }
        h1 {
          font-size: 2em;
          margin-bottom: 20px;
        }
        p {
          font-size: 1.5em;
          margin: 10px 0;
        }
        button {
          padding: 10px 20px;
          margin-top: 20px;
          font-size: 1em;
          border: none;
          border-radius: 5px;
          background-color: #ffffff;
          color: #0072ff;
          cursor: pointer;
        }
        button:hover {
          background-color: #dddddd;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h1>Monitoramento de Ambiente</h1>
        <p>🌡️ Temperatura: <span id="temp">)rawliteral" + String(t, 1) + R"rawliteral(</span> °C</p>
        <p>💧 Umidade: <span id="hum">)rawliteral" + String(h, 1) + R"rawliteral(</span> %</p>
        <button onclick="atualizarDados()">Atualizar Dados</button>
      </div>

      <script>
        function atualizarDados() {
          location.reload();
        }
      </script>
    </body>
    </html>
  )rawliteral";

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

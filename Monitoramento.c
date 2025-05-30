#include <WiFi.h>
#include <WebServer.h>
#include "DHT.h"

// Wi-Fi 
const char* ssid = "Nome_da_sua_rede";
const char* password = "Senha_da_sua_rede";

// DHT22
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Histórico de dados
const int maxLeituras = 10;
float historicoTemperatura[maxLeituras];
float historicoUmidade[maxLeituras];
int indiceLeitura = 0;
int totalLeituras = 0;

// Web server
WebServer server(80);

// Timer
unsigned long ultimoTempoLeitura = 0;
const unsigned long intervaloLeitura = 10000; // 10 segundos

// Página principal
void handleRoot() {
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);  // Garante envio completo do HTML

  String html = R"rawliteral(
    <!DOCTYPE html>
    <html lang="pt-BR">
    <head>
      <meta charset="UTF-8" />
      <meta name="viewport" content="width=device-width, initial-scale=1.0" />
      <title>Monitoramento ESP32</title>
      <style>
        body {
          font-family: Arial, sans-serif;
          background: linear-gradient(to right, #00c6ff, #0072ff);
          color: white;
          padding: 30px;
          text-align: center;
        }
        .container {
          background: rgba(255, 255, 255, 0.1);
          border-radius: 10px;
          padding: 30px;
          margin-bottom: 30px;
          display: inline-block;
        }
        h1 { font-size: 2em; }
        p { font-size: 1.3em; }
      </style>
    </head>
    <body>
      <div class="container">
        <h1>Monitoramento de Ambiente</h1>
        <p id="temp">🌡️ Temperatura Atual: <strong>--°C</strong></p>
        <p id="umid">💧 Umidade Atual: <strong>--%</strong></p>
      </div>

      <h2>📈 Histórico de Leituras</h2>
      <div style="display: flex; justify-content: space-around; flex-wrap: wrap;">
        <canvas id="lineChart" width="600" height="400"></canvas>
        <canvas id="barChart" width="400" height="400"></canvas>
      </div>

      <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
      <script>
        let lineChart, barChart;

        async function atualizarDados() {
          const resposta = await fetch("/dados");
          const json = await resposta.json();
          const temperaturas = json.temperaturas;
          const umidades = json.umidades;

          document.getElementById("temp").innerHTML = "🌡️ Temperatura Atual: <strong>" + temperaturas[temperaturas.length - 1].toFixed(1) + "°C</strong>";
          document.getElementById("umid").innerHTML = "💧 Umidade Atual: <strong>" + umidades[umidades.length - 1].toFixed(1) + "%</strong>";

          const labels = temperaturas.map((_, i) => (i + 1).toString());

          if (lineChart) lineChart.destroy();
          if (barChart) barChart.destroy();

          lineChart = new Chart(document.getElementById("lineChart"), {
            type: "line",
            data: {
              labels: labels,
              datasets: [
                {
                  label: "Temperatura (°C)",
                  data: temperaturas,
                  yAxisID: 'y1',
                  borderColor: "blue",
                  fill: false
                },
                {
                  label: "Umidade (%)",
                  data: umidades,
                  yAxisID: 'y2',
                  borderColor: "red",
                  fill: false
                }
              ]
            },
            options: {
              responsive: false,
              scales: {
                y1: {
                  type: 'linear',
                  position: 'left',
                  title: { display: true, text: 'Temperatura (°C)' }
                },
                y2: {
                  type: 'linear',
                  position: 'right',
                  title: { display: true, text: 'Umidade (%)' },
                  grid: { drawOnChartArea: false }
                }
              }
            }
          });

          const mediaTemp = temperaturas.reduce((a, b) => a + b) / temperaturas.length;
          const mediaUmid = umidades.reduce((a, b) => a + b) / umidades.length;

          barChart = new Chart(document.getElementById("barChart"), {
            type: "bar",
            data: {
              labels: ["Temperatura Média", "Umidade Média"],
              datasets: [{
                label: "Média",
                data: [mediaTemp.toFixed(2), mediaUmid.toFixed(2)],
                backgroundColor: ["blue", "red"]
              }]
            },
            options: {
              responsive: false,
              scales: {
                y: {
                  beginAtZero: true
                }
              }
            }
          });
        }

        setInterval(atualizarDados, 10000);
        window.onload = atualizarDados;
      </script>
    </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", html);
}

// Endpoint para fornecer os dados em JSON
void handleDados() {
  String json = "{ \"temperaturas\": [";
  for (int i = 0; i < totalLeituras; i++) {
    int idx = (indiceLeitura + i) % maxLeituras;
    json += String(historicoTemperatura[idx]);
    if (i < totalLeituras - 1) json += ",";
  }
  json += "], \"umidades\": [";
  for (int i = 0; i < totalLeituras; i++) {
    int idx = (indiceLeitura + i) % maxLeituras;
    json += String(historicoUmidade[idx]);
    if (i < totalLeituras - 1) json += ",";
  }
  json += "] }";

  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/dados", handleDados);
  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void loop() {
  server.handleClient();

  // Atualiza a leitura do sensor a cada 10 segundos
  unsigned long agora = millis();
  if (agora - ultimoTempoLeitura >= intervaloLeitura) {
    ultimoTempoLeitura = agora;

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (!isnan(h) && !isnan(t)) {
      historicoTemperatura[indiceLeitura] = t;
      historicoUmidade[indiceLeitura] = h;
      indiceLeitura = (indiceLeitura + 1) % maxLeituras;
      if (totalLeituras < maxLeituras) totalLeituras++;
    }
  }
}
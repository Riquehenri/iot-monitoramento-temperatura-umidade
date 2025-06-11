#include <WiFi.h>
#include <WebServer.h>
#include "DHT.h"

// Wi-Fi 
const char* ssid = "Felipe";
const char* password = "12345678";

// DHT22
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Histórico de dados (10 leituras a cada 10 segundos)
const int maxLeituras = 10;
float historicoTemperatura[maxLeituras];
float historicoUmidade[maxLeituras];
String timestamps[maxLeituras]; // Para armazenar timestamps
int indiceLeitura = 0;
int totalLeituras = 0;

// Histórico de dados por hora (médias das últimas 10 horas)
const int maxHoras = 10;
float mediasTemperatura[maxHoras];
float mediasUmidade[maxHoras];
int indiceHora = 0;
int totalHoras = 0;
float somaTemperatura = 0;
float somaUmidade = 0;
int leiturasPorHora = 0;
String horas[maxHoras];

// Web server
WebServer server(80);

// Timer
unsigned long ultimoTempoLeitura = 0;
const unsigned long intervaloLeitura = 10000; // 10 segundos
unsigned long ultimaHora = 0;
const unsigned long intervaloHora = 3600000; // 1 hora

// Simulação de data/hora (inicia em 11/06/2025 00:00:00)
unsigned long startMillis = 0;

// Função para gerar timestamp simulado
String getTimestamp(unsigned long currentMillis) {
  unsigned long seconds = (currentMillis - startMillis) / 1000;
  int day = 11; // Fixo para 11/06/2025
  int month = 6;
  int year = 2025;
  int hour = seconds / 3600;
  int minute = (seconds % 3600) / 60;
  int second = seconds % 60;

  // Ajustar overflow do dia
  day += hour / 24;
  hour = hour % 24;

  char buffer[20];
  sprintf(buffer, "%02d/%02d/%04d %02d:%02d:%02d", day, month, year, hour, minute, second);
  return String(buffer);
}

// Página principal
void handleRoot() {
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);

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
        .chart-container { margin: 20px; }
        .log-container {
          background: rgba(255, 255, 255, 0.1);
          border-radius: 10px;
          padding: 20px;
          max-width: 600px;
          margin: 20px auto;
          max-height: 200px;
          overflow-y: auto;
          text-align: left;
        }
        .log-entry { font-size: 1.1em; margin: 5px 0; }
      </style>
    </head>
    <body>
      <div class="container">
        <h1>Monitoramento de Ambiente</h1>
        <p id="temp">🌡️ Temperatura Atual: <strong>--°C</strong></p>
        <p id="umid">💧 Umidade Atual: <strong>--%</strong></p>
      </div>

      <h2>📈 Histórico de Leituras (Últimos 100 segundos)</h2>
      <div style="display: flex; justify-content: space-around; flex-wrap: wrap;">
        <div class="chart-container">
          <canvas id="lineChart" width="600" height="400"></canvas>
        </div>
        <div class="chart-container">
          <canvas id="barChart" width="400" height="400"></canvas>
        </div>
      </div>

      <h2>📅 Histórico por Hora (Últimas 10 horas)</h2>
      <div class="chart-container">
        <canvas id="hourlyChart" width="600" height="400"></canvas>
      </div>

      <h2>📜 Log de Leituras</h2>
      <div class="log-container" id="log">
        <p>Carregando log...</p>
      </div>

      <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
      <script>
        let lineChart, barChart, hourlyChart;

        async function atualizarDados() {
          const resposta = await fetch("/dados");
          const json = await resposta.json();
          const temperaturas = json.temperaturas;
          const umidades = json.umidades;
          const mediasTempHoras = json.mediasTempHoras;
          const mediasUmidHoras = json.mediasUmidHoras;
          const labelsHoras = json.horas;
          const timestamps = json.timestamps;

          document.getElementById("temp").innerHTML = "🌡️ Temperatura Atual: <strong>" + temperaturas[temperaturas.length - 1].toFixed(1) + "°C</strong>";
          document.getElementById("umid").innerHTML = "💧 Umidade Atual: <strong>" + umidades[umidades.length - 1].toFixed(1) + "%</strong>";

          // Atualizar log
          const logDiv = document.getElementById("log");
          logDiv.innerHTML = "";
          for (let i = 0; i < timestamps.length; i++) {
            const entry = document.createElement("p");
            entry.className = "log-entry";
            entry.innerHTML = `Dia ${timestamps[i]} Temperatura: ${temperaturas[i].toFixed(1)}°C Umidade: ${umidades[i].toFixed(1)}%`;
            logDiv.appendChild(entry);
          }

          const labels = temperaturas.map((_, i) => (i + 1).toString());

          if (lineChart) lineChart.destroy();
          if (barChart) barChart.destroy();
          if (hourlyChart) hourlyChart.destroy();

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

          hourlyChart = new Chart(document.getElementById("hourlyChart"), {
            type: "line",
            data: {
              labels: labelsHoras,
              datasets: [
                {
                  label: "Temperatura Média (°C)",
                  data: mediasTempHoras,
                  yAxisID: 'y1',
                  borderColor: "blue",
                  fill: false
                },
                {
                  label: "Umidade Média (%)",
                  data: mediasUmidHoras,
                  yAxisID: 'y2',
                  borderColor: "red",
                  fill: false
                }
              ]
            },
            options: {
              responsive: false,
              scales: {
                x: {
                  title: { display: true, text: 'Hora' }
                },
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
  json += "], \"mediasTempHoras\": [";
  for (int i = 0; i < totalHoras; i++) {
    int idx = (indiceHora + i) % maxHoras;
    json += String(mediasTemperatura[idx]);
    if (i < totalHoras - 1) json += ",";
  }
  json += "], \"mediasUmidHoras\": [";
  for (int i = 0; i < totalHoras; i++) {
    int idx = (indiceHora + i) % maxHoras;
    json += String(mediasUmidade[idx]);
    if (i < totalHoras - 1) json += ",";
  }
  json += "], \"horas\": [";
  for (int i = 0; i < totalHoras; i++) {
    int idx = (indiceHora + i) % maxHoras;
    json += "\"" + horas[idx] + "\"";
    if (i < totalHoras - 1) json += ",";
  }
  json += "], \"timestamps\": [";
  for (int i = 0; i < totalLeituras; i++) {
    int idx = (indiceLeitura + i) % maxLeituras;
    json += "\"" + timestamps[idx] + "\"";
    if (i < totalLeituras - 1) json += ",";
  }
  json += "], \"maxLeituras\": " + String(maxLeituras) + ", \"indiceLeitura\": " + String(indiceLeitura) + " }";

  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  startMillis = millis(); // Inicializar tempo de início

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

  unsigned long agora = millis();
  if (agora - ultimoTempoLeitura >= intervaloLeitura) {
    ultimoTempoLeitura = agora;

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (!isnan(h) && !isnan(t)) {
      // Armazenar leitura de 10 segundos
      historicoTemperatura[indiceLeitura] = t;
      historicoUmidade[indiceLeitura] = h;
      timestamps[indiceLeitura] = getTimestamp(agora);
      indiceLeitura = (indiceLeitura + 1) % maxLeituras;
      if (totalLeituras < maxLeituras) totalLeituras++;

      // Acumular para média horária
      somaTemperatura += t;
      somaUmidade += h;
      leiturasPorHora++;
    }

    // Verificar se passou 1 hora
    if (agora - ultimaHora >= intervaloHora) {
      if (leiturasPorHora > 0) {
        // Calcular médias horárias
        mediasTemperatura[indiceHora] = somaTemperatura / leiturasPorHora;
        mediasUmidade[indiceHora] = somaUmidade / leiturasPorHora;

        // Obter hora atual para a label
        unsigned long segundos = (agora - startMillis) / 1000;
        int hora = (segundos / 3600) % 24;
        int minuto = (segundos % 3600) / 60;
        char horaStr[6];
        sprintf(horaStr, "%02d:%02d", hora, minuto);
        horas[indiceHora] = String(horaStr);

        indiceHora = (indiceHora + 1) % maxHoras;
        if (totalHoras < maxHoras) totalHoras++;

        // Resetar acumuladores
        somaTemperatura = 0;
        somaUmidade = 0;
        leiturasPorHora = 0;
      }
      ultimaHora = agora;
    }
  }
}

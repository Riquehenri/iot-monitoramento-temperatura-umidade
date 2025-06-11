#include <WiFi.h>
#include <WebServer.h>
#include "DHT.h"

// Configuração Wi-Fi
const char* ssid = "Felipe";
const char* password = "12345678";

// Configuração DHT22
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Histórico de dados (leituras a cada 10 segundos)
const int maxLeituras = 10;
float historicoTemperatura[maxLeituras];
float historicoUmidade[maxLeituras];
int indiceLeitura = 0;
int totalLeituras = 0;

// Histórico de dados horários (leituras a cada hora)
const int maxLeiturasHorarias = 10;
float historicoTemperaturaHoraria[maxLeiturasHorarias];
float historicoUmidadeHoraria[maxLeiturasHorarias];
int indiceLeituraHoraria = 0;
int totalLeiturasHorarias = 0;
unsigned long ultimoTempoLeituraHoraria = 0;
const unsigned long intervaloLeituraHoraria = 3600000; // 1 hora em milissegundos

// Web server
WebServer server(80);

// Timer para leituras
unsigned long ultimoTempoLeitura = 0;
const unsigned long intervaloLeitura = 10000; // 10 segundos

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
        .chart-container {
          display: flex;
          justify-content: space-around;
          flex-wrap: wrap;
          margin: 20px 0;
        }
        .chart-wrapper {
          margin: 10px;
          background: rgba(255, 255, 255, 0.1);
          padding: 15px;
          border-radius: 8px;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h1>Monitoramento de Ambiente</h1>
        <p id="temp">🌡️ Temperatura Atual: <strong>--°C</strong></p>
        <p id="umid">💧 Umidade Atual: <strong>--%</strong></p>
      </div>

      <h2>📈 Histórico de Leituras (últimos 10 minutos)</h2>
      <div class="chart-container">
        <div class="chart-wrapper">
          <canvas id="lineChart" width="600" height="400"></canvas>
        </div>
        <div class="chart-wrapper">
          <canvas id="barChart" width="400" height="400"></canvas>
        </div>
      </div>

      <h2>⏳ Histórico Horário (últimas 10 horas)</h2>
      <div class="chart-container">
        <div class="chart-wrapper">
          <canvas id="hourlyLineChart" width="600" height="400"></canvas>
        </div>
      </div>

      <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
      <script>
        let lineChart, barChart, hourlyLineChart;

        async function atualizarDados() {
          try {
            const resposta = await fetch("/dados");
            const json = await resposta.json();
            const temperaturas = json.temperaturas || [];
            const umidades = json.umidades || [];
            const temperaturasHorarias = json.temperaturasHorarias || [];
            const umidadesHorarias = json.umidadesHorarias || [];

            // Atualiza os valores atuais
            document.getElementById("temp").innerHTML = "🌡️ Temperatura Atual: <strong>" + 
              (temperaturas.length > 0 ? temperaturas[temperaturas.length - 1].toFixed(1) : "--") + "°C</strong>";
            document.getElementById("umid").innerHTML = "💧 Umidade Atual: <strong>" + 
              (umidades.length > 0 ? umidades[umidades.length - 1].toFixed(1) : "--") + "%</strong>";

            // Cria labels para os gráficos de 10 minutos
            const labelsMinutos = temperaturas.map((_, i) => (i * 10) + "s");

            // Cria labels para o gráfico horário (formato HH:00)
            const agora = new Date();
            const labelsHorarios = [];
            for (let i = 9; i >= 0; i--) {
              let hora = new Date(agora.getTime() - i * 3600000);
              labelsHorarios.push(hora.getHours().toString().padStart(2, '0') + ':00');
            }

            // Atualiza ou cria os gráficos
            atualizarGraficoLinha(lineChart, "lineChart", labelsMinutos, 
              temperaturas, umidades, "Temperatura (°C)", "blue", "Umidade (%)", "red");
            
            atualizarGraficoBarras(barChart, "barChart", temperaturas, umidades);
            
            atualizarGraficoLinha(hourlyLineChart, "hourlyLineChart", labelsHorarios, 
              temperaturasHorarias, umidadesHorarias, "Temp. Horária (°C)", "green", "Umid. Horária (%)", "orange", 
              "Histórico das Últimas 10 Horas");
          } catch (error) {
            console.error("Erro ao atualizar dados:", error);
          }
        }

        function atualizarGraficoLinha(chart, canvasId, labels, data1, data2, label1, color1, label2, color2, title = "") {
          if (chart) chart.destroy();
          
          chart = new Chart(document.getElementById(canvasId), {
            type: "line",
            data: {
              labels: labels,
              datasets: [
                {
                  label: label1,
                  data: data1,
                  yAxisID: 'y1',
                  borderColor: color1,
                  backgroundColor: color1 + "20",
                  borderWidth: 2,
                  fill: true
                },
                {
                  label: label2,
                  data: data2,
                  yAxisID: 'y2',
                  borderColor: color2,
                  backgroundColor: color2 + "20",
                  borderWidth: 2,
                  fill: true
                }
              ]
            },
            options: {
              responsive: false,
              plugins: {
                title: {
                  display: !!title,
                  text: title
                }
              },
              scales: {
                y1: {
                  type: 'linear',
                  position: 'left',
                  title: { display: true, text: label1 }
                },
                y2: {
                  type: 'linear',
                  position: 'right',
                  title: { display: true, text: label2 },
                  grid: { drawOnChartArea: false }
                }
              }
            }
          });
          return chart;
        }

        function atualizarGraficoBarras(chart, canvasId, temperaturas, umidades) {
          if (chart) chart.destroy();
          
          const mediaTemp = temperaturas.length > 0 ? temperaturas.reduce((a, b) => a + b) / temperaturas.length : 0;
          const mediaUmid = umidades.length > 0 ? umidades.reduce((a, b) => a + b) / umidades.length : 0;

          chart = new Chart(document.getElementById(canvasId), {
            type: "bar",
            data: {
              labels: ["Temperatura Média", "Umidade Média"],
              datasets: [{
                label: "Médias",
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
          return chart;
        }

        // Atualiza os dados a cada 10 segundos
        setInterval(atualizarDados, 10000);
        window.onload = atualizarDados;
      </script>
    </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", html);
}

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
  json += "], \"temperaturasHorarias\": [";
  for (int i = 0; i < totalLeiturasHorarias; i++) {
    int idx = (indiceLeituraHoraria + i) % maxLeiturasHorarias;
    json += String(historicoTemperaturaHoraria[idx]);
    if (i < totalLeiturasHorarias - 1) json += ",";
  }
  json += "], \"umidadesHorarias\": [";
  for (int i = 0; i < totalLeiturasHorarias; i++) {
    int idx = (indiceLeituraHoraria + i) % maxLeiturasHorarias;
    json += String(historicoUmidadeHoraria[idx]);
    if (i < totalLeiturasHorarias - 1) json += ",";
  }
  json += "] }";

  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Conecta ao Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Inicializa arrays com valores padrão
  for (int i = 0; i < maxLeituras; i++) {
    historicoTemperatura[i] = 0;
    historicoUmidade[i] = 0;
  }
  
  for (int i = 0; i < maxLeiturasHorarias; i++) {
    historicoTemperaturaHoraria[i] = 24.0 + (i * 0.1);
    historicoUmidadeHoraria[i] = 50.0 + (i * 0.5);
    totalLeiturasHorarias = maxLeiturasHorarias;
  }

  // Configura as rotas do servidor web
  server.on("/", handleRoot);
  server.on("/dados", handleDados);
  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void loop() {
  server.handleClient();

  unsigned long agora = millis();
  
  // Leituras a cada 10 segundos
  if (agora - ultimoTempoLeitura >= intervaloLeitura) {
    ultimoTempoLeitura = agora;

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (!isnan(h) && !isnan(t)) {
      historicoTemperatura[indiceLeitura] = t;
      historicoUmidade[indiceLeitura] = h;
      indiceLeitura = (indiceLeitura + 1) % maxLeituras;
      if (totalLeituras < maxLeituras) totalLeituras++;
      
      Serial.print("Temperatura: ");
      Serial.print(t);
      Serial.print("°C, Umidade: ");
      Serial.print(h);
      Serial.println("%");
    }
  }

  // Leituras horárias
  if (agora - ultimoTempoLeituraHoraria >= intervaloLeituraHoraria) {
    ultimoTempoLeituraHoraria = agora;

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (!isnan(h) && !isnan(t)) {
      historicoTemperaturaHoraria[indiceLeituraHoraria] = t;
      historicoUmidadeHoraria[indiceLeituraHoraria] = h;
      indiceLeituraHoraria = (indiceLeituraHoraria + 1) % maxLeiturasHorarias;
      if (totalLeiturasHorarias < maxLeiturasHorarias) totalLeiturasHorarias++;
      
      Serial.print("Leitura horária - Temperatura: ");
      Serial.print(t);
      Serial.print("°C, Umidade: ");
      Serial.print(h);
      Serial.println("%");
    }
  }
}

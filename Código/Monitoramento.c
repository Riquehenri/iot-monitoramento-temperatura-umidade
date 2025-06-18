/*
 * SEÇÃO 1: INCLUSÃO DE BIBLIOTECAS
 * Importa bibliotecas necessárias para comunicação Wi-Fi, servidor web, sensor de temperatura e umidade (DHT22),
 * sincronização de tempo (NTP) e integração com o Telegram para envio de mensagens.
 */
#include <WiFi.h>                // Biblioteca para conectar o ESP32 a redes Wi-Fi
#include <WebServer.h>           // Biblioteca para criar um servidor web no ESP32
#include <DHT.h>                // Biblioteca para interface com o sensor DHT22
#include <NTPClient.h>          // Biblioteca para sincronização de tempo via NTP
#include <WiFiUdp.h>            // Biblioteca para comunicação UDP, usada pelo NTP
#include <UniversalTelegramBot.h> // Biblioteca para integração com bots do Telegram
#include <WiFiClientSecure.h>    // Biblioteca para conexões seguras HTTPS com o Telegram

/*
 * SEÇÃO 2: CONFIGURAÇÕES DE REDE E TELEGRAM
 * Define as credenciais do Wi-Fi e as configurações do bot do Telegram, incluindo o token do bot
 * e o ID do grupo ou chat onde as mensagens serão enviadas.
 */
const char* ssid = "SUA_REDE"; // Nome da rede Wi-Fi (SSID)
const char* password = "SUA_SENHA";            // Senha da rede Wi-Fi
#define BOT_TOKEN "8175208512:AAFm10c7iBvhh9zW71rKSeG_akf2BP6_pTA" // Token do bot do Telegram
String chatIds[] = {"-4971691400"};           // Array com IDs de chats/grupos do Telegram
WiFiClientSecure client;                      // Cliente seguro para comunicação HTTPS com o Telegram
UniversalTelegramBot bot(BOT_TOKEN, client);   // Instância do bot do Telegram

/*
 * SEÇÃO 3: CONFIGURAÇÃO DO SENSOR DHT22
 * Define o pino e tipo do sensor DHT22, além dos parâmetros para alertas de temperatura.
 */
#define DHTPIN 4                  // Pino GPIO conectado ao sensor DHT22
#define DHTTYPE DHT22             // Tipo de sensor (DHT22 para maior precisão)
DHT dht(DHTPIN, DHTTYPE);         // Instância do sensor DHT22
const float TEMP_THRESHOLD = 24.5; // Limite de temperatura para envio de alertas (°C)
unsigned long lastAlertTime = 0;  // Timestamp do último alerta enviado (ms)
const unsigned long ALERT_INTERVAL = 60000; // Intervalo mínimo entre alertas (1 minuto, em ms)

/*
 * SEÇÃO 4: ESTRUTURAS DE DADOS
 * Define arrays para armazenar o histórico de leituras, médias horárias e logs de eventos.
 * Usa buffers circulares para gerenciar espaço de forma eficiente.
 */
const int maxLeituras = 10;            // Máximo de leituras armazenadas (10 leituras)
float historicoTemperatura[maxLeituras]; // Array para temperaturas
float historicoUmidade[maxLeituras];    // Array para umidades
String timestamps[maxLeituras];         // Array para timestamps das leituras
int indiceLeitura = 0;                 // Índice atual do buffer de leituras
int totalLeituras = 0;                 // Total de leituras armazenadas

const int maxHoras = 10;               // Máximo de médias horárias armazenadas
float mediasTemperatura[maxHoras];     // Array para médias horárias de temperatura
float mediasUmidade[maxHoras];         // Array para médias horárias de umidade
String horas[maxHoras];                // Array para timestamps das médias horárias
int indiceHora = 0;                    // Índice atual do buffer de médias
int totalHoras = 0;                    // Total de médias horárias armazenadas
float somaTemperatura = 0;             // Soma das temperaturas para cálculo da média
float somaUmidade = 0;                 // Soma das umidades para cálculo da média
int leiturasPorHora = 0;               // Contador de leituras em uma hora

const int maxLog = 10;                 // Máximo de entradas de log
String logEntradas[maxLog];            // Array para armazenar logs
int indiceLog = 0;                     // Índice atual do buffer de logs
int totalLog = 0;                      // Total de entradas de log

/*
 * SEÇÃO 5: CONFIGURAÇÃO DO SERVIDOR WEB E NTP
 * Configura o servidor web na porta 80 e o cliente NTP para sincronização de tempo.
 */
WebServer server(80);                  // Servidor web na porta padrão HTTP (80)
WiFiUDP ntpUDP;                        // Instância UDP para comunicação com servidor NTP
NTPClient timeClient(ntpUDP, "pool.ntp.org", -10800, 60000); // Cliente NTP: servidor pool.ntp.org, fuso horário UTC-3 (-10800s), atualização a cada 60s

/*
 * SEÇÃO 6: CONTROLE DE TEMPORIZAÇÃO
 * Define variáveis para controlar os intervalos de leitura do sensor e cálculo de médias horárias.
 */
unsigned long ultimoTempoLeitura = 0; // Timestamp da última leitura do sensor (ms)
const unsigned long intervaloLeitura = 10000; // Intervalo entre leituras (10 segundos, em ms)
unsigned long ultimaHora = 0;         // Timestamp da última média horária (ms)
const unsigned long intervaloHora = 60000; // Intervalo para médias horárias (1 minuto, em ms)

/*
 * SEÇÃO 7: FUNÇÕES AUXILIARES
 * Funções para formatar o tempo e enviar alertas ao Telegram.
 */
String getFormattedTime() {
  timeClient.update();                // Atualiza o cliente NTP para obter o tempo atual
  unsigned long epochTime = timeClient.getEpochTime(); // Obtém o tempo em segundos desde 1970 (Unix epoch)
  time_t localTime = epochTime;       // Converte para time_t
  struct tm* timeinfo = gmtime(&localTime); // Converte para estrutura de tempo em UTC
  char timeStr[17];                   // Buffer para armazenar a string formatada (DD/MM/YYYY HH:MM)
  strftime(timeStr, sizeof(timeStr), "%d/%m/%Y %H:%M", timeinfo); // Formata a data e hora
  return String(timeStr);             // Retorna a string formatada
}

void sendTelegramAlert(float temperature, float humidity) {
  unsigned long now = millis();        // Obtém o tempo atual em milissegundos
  if (WiFi.status() == WL_CONNECTED && (now - lastAlertTime >= ALERT_INTERVAL)) {
    // Verifica se o Wi-Fi está conectado e se o intervalo mínimo entre alertas foi respeitado
    String message = "⚠️ ALERTA: Temperatura alta! " + String(temperature, 1) + "°C e Umidade " + String(humidity, 1) + "% em " + getFormattedTime();
    // Monta a mensagem de alerta com temperatura, umidade e timestamp
    for (String id : chatIds) {       // Itera sobre os IDs de chats/grupos
      if (bot.sendMessage(id, message, "")) {
        Serial.println("Alerta enviado para o Telegram (ID: " + id + ")!"); // Log de sucesso
      } else {
        Serial.println("Falha ao enviar alerta para o Telegram (ID: " + id + ")."); // Log de falha
      }
    }
    lastAlertTime = now;              // Atualiza o timestamp do último alerta
  }
}

/*
 * SEÇÃO 8: INTERFACE WEB
 * Define a página web servida pelo ESP32, incluindo HTML, CSS e JavaScript para exibir
 * temperatura, umidade, gráficos e logs.
 */
void handleRoot() {
  server.setContentLength(CONTENT_LENGTH_UNKNOWN); // Define tamanho dinâmico para a resposta HTTP
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html lang="pt-BR">
    <head>
      <meta charset="UTF-8" /> <!-- Define codificação de caracteres -->
      <meta name="viewport" content="width=device-width, initial-scale=1.0" /> <!-- Configura responsividade -->
      <title>Monitoramento ESP32</title> <!-- Título da página -->
      <style>
        body {
          font-family: Arial, sans-serif; /* Fonte padrão */
          background: white; /* Fundo branco */
          color: #333; /* Cor do texto */
          padding: 30px; /* Espaçamento interno */
          text-align: center; /* Centraliza texto */
        }
        .container {
          background: #f5f5f5; /* Fundo do contêiner */
          border-radius: 10px; /* Bordas arredondadas */
          padding: 30px; /* Espaçamento interno */
          margin-bottom: 30px; /* Margem inferior */
          display: inline-block; /* Contêiner ajustado ao conteúdo */
          box-shadow: 0 2px 4px rgba(0,0,0,0.1); /* Sombra leve */
        }
        h1 { font-size: 2em; color: #333; } /* Estilo do título principal */
        h2 { color: #333; margin-top: 40px; } /* Estilo dos subtítulos */
        p { font-size: 1.3em; color: #333; } /* Estilo dos parágrafos */
        .chart-container {
          background: #f5f5f5; /* Fundo do gráfico */
          border-radius: 10px; /* Bordas arredondadas */
          padding: 20px; /* Espaçamento interno */
          margin: 20px auto; /* Centraliza com margem */
          box-shadow: 0 2px 4px rgba(0,0,0,0.1); /* Sombra leve */
          width: 600px; /* Largura fixa */
          text-align: center; /* Centraliza texto */
        }
        .hourly-center {
          margin: 20px auto; /* Centraliza horizontalmente */
          display: block; /* Exibe como bloco */
          text-align: center; /* Centraliza texto */
          width: 600px; /* Largura fixa */
        }
        .log-container {
          background: #f5f5f5; /* Fundo do log */
          border-radius: 10px; /* Bordas arredondadas */
          padding: 20px; /* Espaçamento interno */
          max-width: 600px; /* Largura máxima */
          margin: 20px auto; /* Centraliza com margem */
          max-height: 200px; /* Altura máxima */
          overflow-y: auto; /* Barra de rolagem vertical */
          text-align: left; /* Alinhamento à esquerda */
          box-shadow: 0 2px 4px rgba(0,0,0,0.1); /* Sombra leve */
        }
        .log-entry { font-size: 1.1em; margin: 5px 0; color: #333; } /* Estilo das entradas de log */
      </style>
    </head>
    <body>
      <div class="container">
        <h1>Monitoramento de Ambiente</h1> <!-- Título da página -->
        <p id="temp">🌡️ Temperatura Atual: <strong>--°C</strong></p> <!-- Exibe temperatura atual -->
        <p id="umid">💧 Umidade Atual: <strong>--%</strong></p> <!-- Exibe umidade atual -->
      </div>

      <h2>📈 Histórico de Leituras (Últimos 100 segundos)</h2> <!-- Título do gráfico de leituras -->
      <div class="chart-container">
        <canvas id="lineChart" width="600" height="400"></canvas> <!-- Canvas para gráfico de linhas -->
      </div>

      <h2>📅 Histórico por Hora (Últimas 10 horas)</h2> <!-- Título do gráfico de médias horárias -->
      <div class="chart-container hourly-center">
        <canvas id="hourlyChart" width="600" height="400"></canvas> <!-- Canvas para gráfico horário -->
      </div>

      <h2>📜 Log de Leituras</h2> <!-- Título do log -->
      <div class="log-container" id="log">
        <!-- Log será preenchido via JavaScript -->
      </div>

      <script src="https://cdn.jsdelivr.net/npm/chart.js"></script> <!-- Inclui biblioteca Chart.js -->
      <script>
        let lineChart, hourlyChart; // Variáveis para armazenar instâncias dos gráficos

        async function atualizarDados() {
          const resposta = await fetch("/dados"); // Faz requisição ao endpoint /dados
          const json = await resposta.json(); // Converte resposta para JSON
          const temperaturas = json.temperaturas; // Array de temperaturas
          const umidades = json.umidades; // Array de umidades
          const mediasTempHoras = json.mediasTempHoras; // Array de médias horárias de temperatura
          const mediasUmidHoras = json.mediasUmidHoras; // Array de médias horárias de umidade
          const labelsHoras = json.horas; // Array de timestamps horários
          const logs = json.logs; // Array de logs

          // Atualiza temperatura e umidade atuais na página
          document.getElementById("temp").innerHTML = "🌡️ Temperatura Atual: <strong>" + temperaturas[temperaturas.length - 1].toFixed(1) + "°C</strong>";
          document.getElementById("umid").innerHTML = "💧 Umidade Atual: <strong>" + umidades[umidades.length - 1].toFixed(1) + "%</strong>";

          // Atualiza o contêiner de logs
          const logContainer = document.getElementById("log");
          logContainer.innerHTML = ""; // Limpa logs anteriores
          logs.forEach(log => {
            const div = document.createElement("div"); // Cria elemento para cada log
            div.className = "log-entry"; // Aplica estilo
            div.textContent = log; // Define texto do log
            logContainer.appendChild(div); // Adiciona ao contêiner
          });

          // Gera rótulos para o gráfico de leituras (números sequenciais)
          const labels = temperaturas.map((_, i) => (i + 1).toString());

          // Destroi gráficos existentes para evitar sobreposição
          if (lineChart) lineChart.destroy();
          if (hourlyChart) hourlyChart.destroy();

          // Cria gráfico de linhas para histórico de leituras
          lineChart = new Chart(document.getElementById("lineChart"), {
            type: "line", // Tipo de gráfico
            data: {
              labels: labels, // Rótulos do eixo X
              datasets: [
                { label: "Temperatura (°C)", data: temperaturas, yAxisID: 'y1', borderColor: "blue", fill: false }, // Dados de temperatura
                { label: "Umidade (%)", data: umidades, yAxisID: 'y2', borderColor: "red", fill: false } // Dados de umidade
              ]
            },
            options: {
              responsive: false, // Desativa responsividade para tamanho fixo
              scales: {
                y1: { type: 'linear', position: 'left', title: { display: true, text: 'Temperatura (°C)' } }, // Eixo Y para temperatura
                y2: { type: 'linear', position: 'right', title: { display: true, text: 'Umidade (%)' }, grid: { drawOnChartArea: false } } // Eixo Y para umidade
              }
            }
          });

          // Cria gráfico de linhas para médias horárias
          hourlyChart = new Chart(document.getElementById("hourlyChart"), {
            type: "line", // Tipo de gráfico
            data: {
              labels: labelsHoras, // Rótulos do eixo X (horas)
              datasets: [
                { label: "Temperatura Média (°C)", data: mediasTempHoras, yAxisID: 'y1', borderColor: "blue", fill: false }, // Médias de temperatura
                { label: "Umidade Média (%)", data: mediasUmidHoras, yAxisID: 'y2', borderColor: "red", fill: false } // Médias de umidade
              ]
            },
            options: {
              responsive: false, // Desativa responsividade para tamanho fixo
              scales: {
                x: { title: { display: true, text: 'Hora' } }, // Título do eixo X
                y1: { type: 'linear', position: 'left', title: { display: true, text: 'Temperatura (°C)' } }, // Eixo Y para temperatura
                y2: { type: 'linear', position: 'right', title: { display: true, text: 'Umidade (%)' }, grid: { drawOnChartArea: false } } // Eixo Y para umidade
              }
            }
          });
        }

        setInterval(atualizarDados, 10000); // Atualiza dados a cada 10 segundos
        window.onload = atualizarDados; // Executa ao carregar a página
      </script>
    </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", html); // Envia a página HTML como resposta HTTP
}

/*
 * SEÇÃO 9: ENDPOINT JSON
 * Fornece dados em formato JSON para a interface web, incluindo temperaturas, umidades,
 * médias horárias e logs.
 */
void handleDados() {
  String json = "{ \"temperaturas\": ["; // Inicia o objeto JSON com array de temperaturas
  for (int i = 0; i < totalLeituras; i++) {
    int idx = (indiceLeitura + i) % maxLeituras; // Calcula índice no buffer circular
    json += String(historicoTemperatura[idx]); // Adiciona temperatura
    if (i < totalLeituras - 1) json += ","; // Adiciona vírgula, exceto na última entrada
  }
  json += "], \"umidades\": ["; // Inicia array de umidades
  for (int i = 0; i < totalLeituras; i++) {
    int idx = (indiceLeitura + i) % maxLeituras; // Calcula índice no buffer circular
    json += String(historicoUmidade[idx]); // Adiciona umidade
    if (i < totalLeituras - 1) json += ","; // Adiciona vírgula, exceto na última entrada
  }
  json += "], \"mediasTempHoras\": ["; // Inicia array de médias horárias de temperatura
  for (int i = 0; i < totalHoras; i++) {
    int idx = (indiceHora + i) % maxHoras; // Calcula índice no buffer circular
    json += String(mediasTemperatura[idx]); // Adiciona média de temperatura
    if (i < totalHoras - 1) json += ","; // Adiciona vírgula, exceto na última entrada
  }
  json += "], \"mediasUmidHoras\": ["; // Inicia array de médias horárias de umidade
  for (int i = 0; i < totalHoras; i++) {
    int idx = (indiceHora + i) % maxHoras; // Calcula índice no buffer circular
    json += String(mediasUmidade[idx]); // Adiciona média de umidade
    if (i < totalHoras - 1) json += ","; // Adiciona vírgula, exceto na última entrada
  }
  json += "], \"horas\": ["; // Inicia array de timestamps horários
  for (int i = 0; i < totalHoras; i++) {
    int idx = (indiceHora + i) % maxHoras; // Calcula índice no buffer circular
    json += "\"" + horas[idx] + "\""; // Adiciona timestamp formatado
    if (i < totalHoras - 1) json += ","; // Adiciona vírgula, exceto na última entrada
  }
  json += "], \"logs\": ["; // Inicia array de logs
  for (int i = 0; i < totalLog; i++) {
    int idx = (indiceLog + i) % maxLog; // Calcula índice no buffer circular
    json += "\"" + logEntradas[idx] + "\""; // Adiciona entrada de log
    if (i < totalLog - 1) json += ","; // Adiciona vírgula, exceto na última entrada
  }
  json += "] }"; // Fecha o objeto JSON

  server.send(200, "application/json", json); // Envia a resposta JSON
}

/*
 * SEÇÃO 10: CONFIGURAÇÃO INICIAL
 * Inicializa o hardware, conecta ao Wi-Fi, configura o sensor, sincroniza o tempo e inicia o servidor web.
 */
void setup() {
  Serial.begin(115200); // Inicia comunicação serial para depuração
  dht.begin(); // Inicializa o sensor DHT22

  WiFi.begin(ssid, password); // Inicia conexão Wi-Fi com as credenciais fornecidas
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) { // Aguarda conexão
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP()); // Exibe o endereço IP atribuído

  client.setInsecure(); // Configura cliente HTTPS para aceitar certificados não confiáveis (Telegram)

  timeClient.begin(); // Inicia o cliente NTP
  timeClient.update(); // Sincroniza o tempo
  Serial.println("Hora sincronizada: " + getFormattedTime()); // Exibe hora sincronizada

  server.on("/", handleRoot); // Associa a rota "/" à função handleRoot
  server.on("/dados", handleDados); // Associa a rota "/dados" à função handleDados
  server.begin(); // Inicia o servidor web
  Serial.println("Servidor HTTP iniciado"); // Confirma inicialização
}

/*
 * SEÇÃO 11: LOOP PRINCIPAL
 * Executa continuamente para ler o sensor, enviar alertas, atualizar históricos e gerenciar o servidor web.
 */
void loop() {
  server.handleClient(); // Processa requisições HTTP recebidas

  unsigned long agora = millis(); // Obtém o tempo atual em milissegundos
  if (agora - ultimoTempoLeitura >= intervaloLeitura) { // Verifica se é hora de nova leitura
    ultimoTempoLeitura = agora; // Atualiza timestamp da última leitura

    float h = dht.readHumidity(); // Lê umidade do sensor
    float t = dht.readTemperature(); // Lê temperatura do sensor

    if (!isnan(h) && !isnan(t)) { // Verifica se as leituras são válidas
      historicoTemperatura[indiceLeitura] = t; // Armazena temperatura
      historicoUmidade[indiceLeitura] = h; // Armazena umidade
      timestamps[indiceLeitura] = getFormattedTime(); // Armazena timestamp
      indiceLeitura = (indiceLeitura + 1) % maxLeituras; // Avança índice (buffer circular)
      if (totalLeituras < maxLeituras) totalLeituras++; // Incrementa total de leituras

      String logStr = getFormattedTime() + " Temperatura " + String(t, 1) + "°C / Umidade " + String(h, 1) + "%"; // Cria entrada de log
      logEntradas[indiceLog] = logStr; // Armazena log
      indiceLog = (indiceLog + 1) % maxLog; // Avança índice (buffer circular)
      if (totalLog < maxLog) totalLog++; // Incrementa total de logs

      if (t > TEMP_THRESHOLD) { // Verifica se a temperatura excede o limite
        Serial.printf("Temperatura: %.1f°C, Umidade: %.1f%%, enviando alerta...\n", t, h); // Log de alerta
        sendTelegramAlert(t, h); // Envia alerta ao Telegram
      }

      somaTemperatura += t; // Acumula temperatura para média
      somaUmidade += h; // Acumula umidade para média
      leiturasPorHora++; // Incrementa contador de leituras por hora
    }

    if (agora - ultimaHora >= intervaloHora) { // Verifica se é hora de calcular média horária
      if (leiturasPorHora > 0) { // Se houver leituras na hora
        mediasTemperatura[indiceHora] = somaTemperatura / leiturasPorHora; // Calcula média de temperatura
        mediasUmidade[indiceHora] = somaUmidade / leiturasPorHora; // Calcula média de umidade
        horas[indiceHora] = getFormattedTime().substring(11, 16); // Armazena hora (HH:MM)
        indiceHora = (indiceHora + 1) % maxHoras; // Avança índice (buffer circular)
        if (totalHoras < maxHoras) totalHoras++; // Incrementa total de horas
        somaTemperatura = 0; // Reseta acumulador de temperatura
        somaUmidade = 0; // Reseta acumulador de umidade
        leiturasPorHora = 0; // Reseta contador de leituras
      }
      ultimaHora = agora; // Atualiza timestamp da última média
    }
  }
}

# 🌡️ ESP32 IoT Monitor - Temperatura e Umidade

Projeto de sistema embarcado com **ESP32** e **sensor DHT22**, que realiza a leitura de temperatura e umidade e exibe os dados em tempo real em uma **interface web interativa**, acessível via rede Wi-Fi.

## 🔧 Tecnologias Utilizadas

- **ESP32**
- **Sensor DHT22**
- **Arduino IDE**
- **HTML/CSS e JavaScript (Chart.js)**
- **Bibliotecas:** `WiFi.h`, `WebServer.h`, `DHT.h`

## 🔌 Conexões do Hardware

| Componente | Pino ESP32 |
|------------|------------|
| DHT22 VCC  | 3V3        |
| DHT22 GND  | GND        |
| DHT22 DATA | GPIO 4     |

## 🌐 Interface Web

Ao conectar o ESP32 ao Wi-Fi, ele exibe no monitor serial um IP local. Basta acessá-lo via navegador para visualizar o painel.

### Funcionalidades da Página

- 🔄 **Atualização Automática:** A cada 10 segundos, os dados são atualizados via requisição assíncrona (AJAX).
- 📈 **Gráfico de Linha:** Mostra as últimas 10 leituras de temperatura e umidade.
- 📊 **Gráfico de Barras:** Apresenta a média recente de temperatura e umidade.
- 📌 **Indicadores Atuais:** Destaques para os valores mais recentes.

> A renderização gráfica é feita com a biblioteca **Chart.js**, carregada diretamente do CDN.

## 🖼️ Diagrama / Arquitetura

<div style="display: flex; gap: 10px;">
  <img src="https://github.com/user-attachments/assets/89dc45e3-1fe6-49c6-83e7-d2e13806ef8a" alt="Diagrama 1" width="48%" />
  <img src="https://github.com/user-attachments/assets/73a3a912-4002-47c1-8b5a-966f0be9b836" alt="Diagrama 2" width="48%" />
</div>

## 📚 Referências

- [ESP32 WiFi Documentation](https://docs.espressif.com/)
- [DHT22 Datasheet - Adafruit](https://www.adafruit.com/product/385)
- [Arduino WebServer Library](https://www.arduino.cc/en/Reference/WebServer)
- [Chart.js (Gráficos Web)](https://www.chartjs.org/)

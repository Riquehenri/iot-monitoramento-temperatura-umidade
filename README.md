# 🌡️ ESP32 IoT Monitor - Temperatura e Umidade

Projeto de sistema embarcado com _ESP32_ e _sensor DHT22_, que realiza a leitura de temperatura e umidade, exibe os dados em tempo real em uma **interface web interativa** acessível via rede Wi-Fi, e envia alertas via Telegram quando a temperatura excede um limite configurado.

## 🔧 Tecnologias Utilizadas

- **Hardware:**

  - _ESP32_ (microcontrolador com Wi-Fi e Bluetooth)
  - _Sensor DHT22_ (sensor digital de temperatura e umidade)
  - Protoboard e jumpers
  - Fonte USB 5V (200-500 mA)

- **Software e Ferramentas:**

  - _Arduino IDE_ (programação do ESP32)
  - _Wokwi_ (simulador online para validação de código)
  - _HTML/CSS e JavaScript_ (interface web com _Chart.js_)
  - _NTPClient_ (sincronização de tempo via NTP)
  - _UniversalTelegramBot_ (integração com Telegram)

- **Bibliotecas:**
  - `WiFi.h`: Gerencia conexões Wi-Fi
  - `WebServer.h`: Implementa servidor HTTP
  - `DHT.h`: Interface com o sensor DHT22
  - `NTPClient.h`: Sincronização de tempo
  - `UniversalTelegramBot.h`: Envio de alertas via Telegram
  - `WiFiClientSecure.h`: Comunicação HTTPS

## 🔌 Conexões do Hardware

| Componente | Pino ESP32 |
| ---------- | ---------- |
| DHT22 VCC  | 3V3        |
| DHT22 GND  | GND        |
| DHT22 DATA | GPIO 4     |

## 🌐 Interface Web

Ao conectar o ESP32 à rede Wi-Fi, ele exibe o endereço IP local no monitor serial. Acesse esse IP via navegador para visualizar o painel interativo.

### Funcionalidades da Página

- **Atualização Automática:** Dados atualizados a cada 10 segundos via requisições AJAX.
- **Gráfico de Linha:** Exibe as últimas 10 leituras de temperatura e umidade (100 segundos).
- **Indicadores Atuais:** Mostra os valores mais recentes de temperatura e umidade em destaque.
- **Log de Leituras:** Exibe um histórico de até 10 leituras com timestamps, formatado em um contêiner com barra de rolagem.
- **Renderização Gráfica:** Utiliza _Chart.js_ (carregado via CDN) para gráficos interativos.

## 🔔 Alertas via Telegram

- **Funcionalidade:** Envia alertas para um grupo ou chat do Telegram quando a temperatura excede 24.5°C.
- **Intervalo:** Alertas são limitados a um por minuto para evitar spam.
- **Formato da Mensagem:** Inclui temperatura, umidade e timestamp formatado (ex.: "⚠️ ALERTA: Temperatura alta! 25.5°C e Umidade 50.0% em 17/06/2025 22:43").
- **Configuração:** Requer o token do bot e o ID do chat/grupo no código.

## ⏰ Sincronização de Tempo

- Utiliza o protocolo NTP (servidor `pool.ntp.org`, fuso horário UTC-3) para sincronizar o tempo.
- Timestamps são formatados como `DD/MM/YYYY HH:MM` para logs e gráficos.

## 📊 Arquitetura do Sistema

O sistema integra o sensor DHT22 ao ESP32, que lê os dados a cada 10 segundos e os disponibiliza via Wi-Fi em uma página HTML. Os dados são armazenados em buffers circulares para histórico (10 leituras) e médias horárias (10 horas). Alertas são enviados via Telegram em caso de temperatura alta. A interface web é gerada pelo ESP32, com gráficos renderizados por _Chart.js_.

<div style="display: flex; gap: 10px;">
  <img src="https://github.com/user-attachments/assets/89dc45e3-1fe6-49c6-83e7-d2e13806ef8a" alt="Diagrama 1" width="48%" />
  <img src="https://github.com/user-attachments/assets/73a3a912-4002-47c1-8b5a-966f0be9b836" alt="Diagrama 2" width="48%" />
</div>

## 🚀 Instruções de Uso

1. **Configurar o Hardware:**

   - Conecte o sensor DHT22 ao ESP32 conforme a tabela de conexões.
   - Alimente o circuito via USB (5V).

2. **Configurar o Software:**

   - Instale as bibliotecas listadas no _Arduino IDE_.
   - Atualize as credenciais Wi-Fi (`ssid` e `password`) no código.
   - Configure o token do bot Telegram e o ID do chat/grupo.

3. **Carregar o Código:**

   - Use o _Arduino IDE_ para carregar o código no ESP32.
   - Abra o monitor serial (115200 baud) para verificar o IP atribuído.

4. **Acessar a Interface:**

   - Conecte-se à mesma rede Wi-Fi do ESP32.
   - Acesse o IP exibido no monitor serial via navegador.

5. **Receber Alertas:**
   - Certifique-se de que o ESP32 está conectado à internet.
   - Alertas serão enviados ao Telegram quando a temperatura exceder 24.5°C.

## 📈 Resultados

- **Precisão:** O sensor DHT22 apresentou leituras consistentes (temperatura: 20-30°C, umidade: 40-60%) em testes controlados.
- **Desempenho:** Latência de resposta da interface web inferior a 1 segundo na rede local.
- **Consumo:** Aproximadamente 200 mA, viável para alimentação USB.
- **Limitações:** Interface básica, operação restrita a redes locais, tempo de resposta do DHT22 (2s) pode limitar aplicações críticas.
- **Melhorias Futuras:** Integração com MQTT, armazenamento em nuvem (Firebase), gráficos históricos mais robustos, alertas configuráveis.

## 📚 Referências

- [ESP32 Technical Reference Manual](https://www.espressif.com/en/products/socs/esp32/resources)
- [DHT22 Datasheet](https://www.sparkfun.com/datasheets/Sensors/Temperature/DHT22.pdf)
- [Arduino WebServer Library](https://www.arduino.cc/en/Reference/WebServer)
- [Chart.js Documentation](https://www.chartjs.org/)
- [NTPClient Library](https://github.com/arduino-libraries/NTPClient)
- [UniversalTelegramBot Library](https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot)
- Atzori, L.; Iera, A.; Morabito, G. _The Internet of Things: A survey_. Computer Networks, v. 54, n. 15, p. 2787-2805, 2010.

# 🌡️ ESP32 IoT Monitor - Temperatura e Umidade

Projeto de sistema embarcado com ESP32 + sensor DHT22, que monitora temperatura e umidade e exibe os dados em tempo real numa interface web via Wi-Fi.

## 🔧 Tecnologias
- ESP32
- Sensor DHT22
- Arduino IDE
- HTML/CSS (Web Server)
- Biblioteca: `WiFi.h`, `WebServer.h`, `DHT.h`

## 🔌 Conexões do Hardware
| Componente | ESP32 |
|------------|-------|
| DHT22 VCC  | 3V3   |
| DHT22 GND  | GND   |
| DHT22 DATA | GPIO 26 |

## 🌐 Interface Web
Ao conectar, o ESP32 fornece um IP local. Acesse esse IP no navegador para visualizar os dados atualizados.

## 📷 Imagem do Circuito / Arquitetura
![Arquitetura](docs/arquitetura.png)

## 📚 Referências
- [ESP32 WiFi docs](https://docs.espressif.com/)
- [DHT22 datasheet](https://www.adafruit.com/product/385)
- [Arduino WebServer Docs](https://www.arduino.cc/en/Reference/WebServer)



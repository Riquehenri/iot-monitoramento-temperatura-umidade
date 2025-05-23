# 🌡️ ESP32 IoT Monitor - Temperatura e Umidade

Projeto de sistema embarcado com ESP32 + sensor DHT22, que monitora temperatura e umidade e exibe os dados em tempo real numa interface web via Wi-Fi.

## 🔧 Tecnologias
- ESP32
- Sensor DHT22
- Arduino IDE
- HTML/CSS (Servidor Web)
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
![Image](https://github.com/user-attachments/assets/89dc45e3-1fe6-49c6-83e7-d2e13806ef8a)

## 📚 Referências
- [ESP32 WiFi docs](https://docs.espressif.com/)
- [DHT22 datasheet](https://www.adafruit.com/product/385)
- [Arduino WebServer Docs](https://www.arduino.cc/en/Reference/WebServer)



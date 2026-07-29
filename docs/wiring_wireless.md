# 无线温度传感器 — 接线指南

## 系统架构

```
[发送端]                           [接收端]
STM32 + DS18B20 + NRF24L01  ──►   ESP32 + NRF24L01  ──WiFi──►  手机 APP
```

---

## 发送端：STM32 接线

### NRF24L01 → STM32

| NRF24L01 | STM32 引脚 | 说明 |
|----------|-----------|------|
| VCC | **3.3V** | 必须是 3.3V！绝对不能接 5V |
| GND | GND | |
| SCK | PA5 | SPI 时钟 |
| MISO | PA6 | SPI 主入从出 |
| MOSI | PA7 | SPI 主出从入 |
| CSN | PB0 | SPI 片选 |
| CE | PB1 | NRF 使能 |

### DS18B20 → STM32（同有线阶段）

| DS18B20 | STM32 |
|---------|-------|
| VDD（红） | 3.3V |
| DQ（黄） | PB12 |
| GND（黑） | GND |

> DQ 与 3.3V 之间需要 4.7kΩ 上拉电阻，模块自带则不需要。

---

## 接收端：ESP32 接线

| NRF24L01 | ESP32 | 说明 |
|----------|-------|------|
| VCC | **3.3V** | 必须是 3.3V |
| GND | GND | |
| SCK | GPIO18 | SPI 时钟 |
| MISO | GPIO19 | SPI 主入从出 |
| MOSI | GPIO23 | SPI 主出从入 |
| CSN | GPIO5 | SPI 片选 |
| CE | GPIO4 | NRF 使能 |

> ESP32 的 3.3V 电流有限，NRF24L01 发送时峰值电流约 12mA，可以直接从 ESP32 供电。

---

## 供电说明

- STM32 发送端：通过 USB（ST-Link 或移动电源）供电
- ESP32 接收端：通过 USB（Micro-USB）供电，可插充电宝
- NRF24L01 **必须用 3.3V**，5V 会烧毁模块

---

## 通电顺序

1. 先接好所有线
2. STM32 上电（程序开始发送温度数据）
3. ESP32 上电（创建 WiFi 热点，开始接收）
4. 手机连接 WiFi `TempMonitor`（密码 `12345678`）
5. 打开 APP 或浏览器访问 `http://192.168.4.1`

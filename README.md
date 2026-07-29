# DS18B20 无线温度监控系统

STM32F103C8T6 + ESP32 + DS18B20 防水探头，实时温度通过 WiFi 传到手机 APP。

## 效果展示

| 硬件连接 | APP 测温 |
|----------|----------|
| ![](docs/images/hardware.jpg) | ![](docs/images/app.jpg) |

### 演示视频

- [手机 APP 测温效果](docs/app_demo.mp4)
- [最小系统板调试检测](docs/debug_test.mp4)

## 功能

- DS18B20 防水探头测温（12-bit，±0.5°C）
- ESP32 自建 WiFi 热点，手机直连，不需要路由器
- Android APP：实时温度、折线图、MAX/MIN、水温/气温切换
- 超过阈值温度数字变红报警
- Python PC 端调试工具

## 系统结构

```
DS18B20 → STM32 → USART2(PA2) → ESP32 → WiFi → 手机 APP
```

STM32 负责读温度，每秒发 `T:25.3\r\n` 给 ESP32。ESP32 收串口数据、开 WiFi 热点、提供 HTTP API。手机 APP 轮询 API 显示温度。

## 硬件接线

**STM32 端**

| STM32 | 连接 |
|-------|------|
| PB12 | DS18B20 DQ（4.7kΩ 上拉） |
| PA2 | ESP32 GPIO16 |
| GND | ESP32 GND |

**DS18B20 探头**：红→3.3V，黄→PB12，黑→GND

## 烧录

1. STM32：Keil 打开 `firmware/Project/TempMonitor.uvprojx`，F7 编译，F8 烧录
2. ESP32：Arduino IDE 打开 `esp32_receiver/esp32_receiver.ino`，上传
3. 手机连 WiFi `TempMonitor`（密码 12345678），打开 APP

## 目录

```
├── firmware/          # STM32 Keil 工程
│   └── User/          # ds18b20, onewire, usart, delay
├── esp32_receiver/    # ESP32 Arduino 代码
├── android_app/       # Android APP
├── app/               # Python 串口工具
└── docs/images/       # 演示图片
```

## 关于 NRF24L01

最初设计用 NRF24L01 做全无线（STM32→NRF→ESP32），发射端已验证正常。ESP32 接收端 SPI 通信不稳定，遂改串口桥接。NRF 驱动保留在 `firmware/User/nrf24l01.c`，后续可换 Arduino Nano 接收端切回纯无线。

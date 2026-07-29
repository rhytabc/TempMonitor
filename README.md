# TempMonitor — DS18B20 无线温度监控系统

[![Platform](https://img.shields.io/badge/Platform-STM32F103-blue)]()
[![WiFi](https://img.shields.io/badge/WiFi-ESP32-orange)]()
[![Sensor](https://img.shields.io/badge/Sensor-DS18B20-green)]()
[![License](https://img.shields.io/badge/License-MIT-lightgrey)]()

基于 STM32F103C8T6 + ESP32 + DS18B20 防水探头的嵌入式温度监控系统，实时测温并通过 WiFi 传到手机 APP。

---

## 效果展示

| 硬件连接 | APP 测温 |
|----------|----------|
| ![](docs/images/hardware.jpg) | ![](docs/images/app.jpg) |

### 演示视频

- [手机 APP 测温效果](docs/app_demo.mp4)
- [最小系统板调试检测](docs/debug_test.mp4)

---

## 功能特性

-  DS18B20 防水探头，12-bit 精度 ±0.5°C
-  STM32F103C8T6 裸机驱动，手写 OneWire 协议
-  ESP32 自建 WiFi AP（`TempMonitor`），无需路由器
-  Android APP：实时温度、折线图、MAX/MIN 记录、水温/气温模式切换
-  超阈值红色报警
-  RESTful API：`GET /temp` → `{"temperature":25.3}`
-  Python PC 端调试工具

## 系统架构

```
DS18B20 ──OneWire──► STM32F103C8T6 ──USART2──► ESP32 ──WiFi AP──► 手机 APP
   ↑                     ↑                        ↑
 防水探头             72MHz 裸机              Arduino 框架
 PB12(Pull-up)       协作式轮询              HTTP Server :80
```

| 芯片 | 职责 |
|------|------|
| STM32F103C8T6 | DS18B20 温度采集（OneWire），每秒通过 USART2(PA2) 发送 `T:25.3\r\n` |
| ESP32 | 串口接收温度数据，WiFi AP 热点（`TempMonitor` / `12345678`），HTTP API |
| Android APP | HTTP 轮询温度，实时显示、曲线图、模式切换、报警 |

## 数据流

```
DS18B20 → STM32 (OneWire) → PA2(TX) → ESP32 GPIO16(RX) → WiFi → 手机 APP
   12-bit ADC           printf          115200 bps       HTTP JSON
   750ms 转换          "T:25.3\r\n"                     /temp
```

## 硬件接线

**STM32 端**

| STM32 | 连接 |
|-------|------|
| PB12 | DS18B20 DQ（4.7kΩ 上拉到 3.3V） |
| PA2 (USART2_TX) | ESP32 GPIO16 |
| GND | ESP32 GND（共地） |

**DS18B20 防水探头**

| 线色 | 连接 |
|------|------|
| 红 | 3.3V |
| 黄 | PB12 |
| 黑 | GND |

## 快速开始

### 前置要求

| 组件 | 说明 |
|------|------|
| 硬件 | STM32F103C8T6 最小系统板、ESP32 DevKit、DS18B20 防水探头、4.7kΩ 电阻 |
| STM32 工具链 | Keil MDK-ARM v5 + STM32F1xx 设备包 |
| ESP32 工具链 | Arduino IDE + ESP32 开发板包 |
| 调试 | ST-Link（烧录 STM32）、USB-TTL（调试用） |

### 烧录

1. **STM32**：Keil 打开 `firmware/Project/TempMonitor.uvprojx` → F7 编译 → F8 烧录
2. **ESP32**：Arduino IDE 打开 `esp32_receiver/esp32_receiver.ino` → 上传
3. **手机**：连接 WiFi `TempMonitor`（密码 `12345678`）→ 安装 APK → 打开 APP

## 项目结构

```
TempMonitor/
├── firmware/                          # STM32 Keil5 工程
│   ├── User/                          # 源文件
│   │   ├── main.c                     # 主程序（温度采集 + 串口发送）
│   │   ├── ds18b20.c/h                # DS18B20 OneWire 驱动
│   │   ├── onewire.c/h                # 1-Wire 底层协议
│   │   ├── delay.c/h                  # DWT 微秒级延时
│   │   ├── usart.c/h                  # USART1+USART2 串口驱动
│   │   ├── spi.c/h                    # SPI 驱动
│   │   └── nrf24l01.c/h               # NRF24L01 驱动（实验）
│   └── Project/                       # Keil 工程文件
├── esp32_receiver/                    # ESP32 Arduino 代码
│   └── esp32_receiver.ino             # 串口接收 + WiFi AP + HTTP API
├── android_app/                       # Android APP (Android Studio)
│   └── app/src/main/
│       ├── java/com/tempmonitor/      # Java 源码
│       │   ├── MainActivity.java      # 主界面
│       │   ├── TempService.java       # HTTP 轮询
│       │   ├── ChartView.java         # 温度曲线控件
│       │   └── TempDescHelper.java    # 温度描述
│       └── res/layout/                # 界面布局
├── app/                               # Python PC 调试工具
│   ├── main.py
│   ├── serial_handler.py
│   └── ui.py
├── docs/                              # 文档 & 演示素材
│   ├── images/hardware.jpg
│   ├── app_demo.mp4
│   └── debug_test.mp4
└── README.md
```

## API 参考

### GET /temp

返回最新温度读数。

**Response 200 OK:**

```json
{
  "temperature": 26.3
}
```

### GET /

打开 Web 仪表盘，浏览器中实时显示温度和系统状态。

## 技术要点

### OneWire 时序

DS18B20 需要微秒级精确时序（1-480μs）。用 DWT（Data Watchpoint and Trace）周期计数器实现 `Delay_us()`——72MHz 主频下每周期约 14ns，1μs = 72 个计数周期，精度 ±1μs。

### 为什么不用 RTOS

系统只有两个周期性任务（读传感器 + 发串口），抢占式 RTOS 的开销不划算。主循环基于 `DWT->CYCCNT` 计数的协作式轮询，零额外开销。

### 时钟配置

- HSE 8MHz 外部晶振 → PLL×9 → SYSCLK 72MHz
- AHB = 72MHz, APB2 = 72MHz, APB1 = 36MHz
- USART 波特率：115200 bps（APB2 72MHz，USARTDIV = 39.0625）

## NRF24L01 无线扩展

本项目最初设计为 NRF24L01 全无线方案：STM32 → NRF → ESP32 → WiFi → APP。发射端在 STM32 上已验证正常（SPI1 + NRF24L01 TX），双模块互通测试通过。ESP32 接收端因 SPI 驱动能力问题暂用串口桥接替代。

NRF 驱动代码保留在 `firmware/User/nrf24l01.c`，后续可换 Arduino Nano/UNO 接收端实现全无线。

# Hardware Wiring Guide

## Components Required

| Component | Quantity | Notes |
|-----------|----------|-------|
| STM32F103C8T6 minimum system board | 1 | Blue Pill or similar |
| DS18B20 temperature sensor | 1 | TO-92 package (3-pin) or waterproof probe |
| 4.7kΩ resistor | 1 | Pull-up for OneWire data line |
| USB-TTL adapter | 1 | CH340/CP2102 (if not built into the board) |
| Breadboard + jumper wires | - | For prototyping |

## Pin Connections

### DS18B20 → STM32

```
DS18B20 Pinout (TO-92, flat side facing you):
   ┌───┐
   │ 1 │  GND (left)
   │ 2 │  DQ  (center) — Data
   │ 3 │  VDD (right) — 3.3V or 5V
   └───┘
```

| DS18B20 Pin | STM32 Pin | Wire Color (typical probe) |
|-------------|-----------|---------------------------|
| VDD (pin 3) | 3.3V | Red |
| DQ (pin 2)  | PB12 | Yellow / White |
| GND (pin 1) | GND | Black |

**Pull-up resistor**: Connect 4.7kΩ between DQ (PB12) and 3.3V.

### USART → USB-TTL

| STM32 Pin | USB-TTL Adapter |
|-----------|----------------|
| PA9 (TX)  | RXD |
| PA10 (RX) | TXD |
| GND       | GND |

### Complete Wiring Diagram

```
     STM32F103C8T6              DS18B20
    ┌──────────────┐          ┌─────────┐
    │              │          │  ┌───┐  │
    │          PB12├──────────┤2 │ ○ │  │ DQ
    │              │  4.7kΩ   │  └───┘  │
    │          3.3V├───███────┤3 │ ○ │  │ VDD
    │              │          │  └───┘  │
    │           GND├──────────┤1 │ ○ │  │ GND
    │              │          │  └───┘  │
    │           PA9├─────┐    └─────────┘
    │          PA10├───┐ │
    │           GND├─┐ │ │    USB-TTL
    │              │ │ │ │  ┌──────────┐
    │           PC13├─LED  │  │ RXD ◄────┤ PA9
    └──────────────┘ │ │ │  │ TXD ──────┤ PA10
       Onboard LED   │ │ │  │ GND ──────┤ GND
                     │ │ └──┤ VCC       │
    ┌────────────────┘ └────┤           │
    │                  ┌───┤ USB ──────► PC
    │  ┌───────────┐   │   └──────────┘
    └──┤ 4.7kΩ     ├───┘
       └───────────┘
```

## Notes

1. **Power**: DS18B20 works with 3.3V (from STM32 board). Do NOT connect to 5V.
2. **Pull-up is required**: The OneWire bus is open-drain and needs the 4.7kΩ pull-up resistor to 3.3V to function.
3. **Parasitic power mode NOT used**: DS18B20 is powered normally (VDD connected). If using parasitic mode (DS18B20 powered from DQ line), only 2 wires are needed but timing is more critical.
4. **Cable length**: Keep OneWire cable under 5 meters for reliable operation. Longer runs may need a stronger pull-up.

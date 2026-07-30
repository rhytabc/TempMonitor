/**
 * NRF24L01 SPI 最简测试 — 只看模块能不能通信
 */
#include <SPI.h>

#define CSN 25
#define CE  26

void setup() {
    Serial.begin(115200);
    pinMode(CSN, OUTPUT);
    pinMode(CE, OUTPUT);
    digitalWrite(CSN, HIGH);
    digitalWrite(CE, LOW);

    // 换到不常用的 GPIO 脚，避免冲突
    SPI.begin(14, 12, 13, 25);  // SCK=14, MISO=12, MOSI=13, CS=25
    SPI.setFrequency(500000);
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);

    Serial.println("NRF24L01 SPI Test (alt pins)");
    Serial.printf("SCK=14 MISO=12 MOSI=13 CSN=25 CE=26\n");
}

void loop() {
    // 读 STATUS 寄存器 (0x07)
    digitalWrite(CSN, LOW);
    SPI.transfer(0x07); // 读 STATUS
    uint8_t status = SPI.transfer(0xFF); // 读
    digitalWrite(CSN, HIGH);

    // 读 CONFIG 寄存器 (0x00)
    digitalWrite(CSN, LOW);
    SPI.transfer(0x00);
    uint8_t config = SPI.transfer(0xFF);
    digitalWrite(CSN, HIGH);

    Serial.printf("STATUS=0x%02X  CONFIG=0x%02X", status, config);

    if (status == 0xFF && config == 0xFF) {
        Serial.println("  <- 全FF，模块没接/没电！");
    } else if (status == 0x00 && config == 0x00) {
        Serial.println("  <- 全00，接线可能有短路");
    } else if ((status & 0x0E) != 0x0E || config == 0x00 || config == 0xFF) {
        Serial.println("  <- 有响应但异常，检查接线");
    } else {
        Serial.println("  <- OK！模块正常工作！");
    }
    delay(1000);
}

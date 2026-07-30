/**
 * NRF24L01 双模块互通测试
 * TX: SPI1 (PA5/6/7, PB0=CSN, PB1=CE)
 * RX: SPI2 (PB13=SCK, PB14=MISO, PB15=MOSI, PB10=CSN, PB11=CE)
 *
 * TX 每秒发 [0xAA][0x12][0x34][0x55]，RX 收到就打印
 */
#include "main.h"
#include "delay.h"
#include "usart.h"
#include "spi.h"
#include <stdio.h>

/* ===== NRF 命令/寄存器 ===== */
#define R_REG       0x00
#define W_REG       0x20
#define R_RX_PL     0x61
#define W_TX_PL     0xA0
#define FLUSH_TX    0xE1
#define FLUSH_RX    0xE2
#define NOP         0xFF

#define CONFIG      0x00
#define EN_AA       0x01
#define EN_RXADDR   0x02
#define SETUP_AW    0x03
#define SETUP_RETR  0x04
#define RF_CH       0x05
#define RF_SETUP    0x06
#define STATUS      0x07
#define RX_ADDR_P0  0x0A
#define TX_ADDR     0x10
#define RX_PW_P0    0x11

/* ===== RX 模块引脚 (PB10=CSN, PB11=CE, SPI2) ===== */
#define RX_CSN_L()  GPIO_ResetBits(GPIOB, GPIO_Pin_10)
#define RX_CSN_H()  GPIO_SetBits(GPIOB, GPIO_Pin_10)
#define RX_CE_L()   GPIO_ResetBits(GPIOB, GPIO_Pin_11)
#define RX_CE_H()   GPIO_SetBits(GPIOB, GPIO_Pin_11)

const uint8_t ch = 100;
const uint8_t addr[5] = {'T','E','S','T','!'};

/* ===== SPI2 操作 ===== */
void SPI2_Init(void) {
    GPIO_InitTypeDef g;
    SPI_InitTypeDef  s;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

    g.GPIO_Speed = GPIO_Speed_50MHz;
    g.GPIO_Pin = GPIO_Pin_13; g.GPIO_Mode = GPIO_Mode_AF_PP; GPIO_Init(GPIOB, &g); // SCK
    g.GPIO_Pin = GPIO_Pin_15; g.GPIO_Mode = GPIO_Mode_AF_PP; GPIO_Init(GPIOB, &g); // MOSI
    g.GPIO_Pin = GPIO_Pin_14; g.GPIO_Mode = GPIO_Mode_IN_FLOATING; GPIO_Init(GPIOB, &g); // MISO
    g.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11; g.GPIO_Mode = GPIO_Mode_Out_PP; GPIO_Init(GPIOB, &g);

    RX_CSN_H(); RX_CE_L();

    SPI_StructInit(&s);
    s.SPI_Mode = SPI_Mode_Master; s.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    s.SPI_DataSize = SPI_DataSize_8b; s.SPI_CPOL = SPI_CPOL_Low; s.SPI_CPHA = SPI_CPHA_1Edge;
    s.SPI_NSS = SPI_NSS_Soft; s.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    s.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_Init(SPI2, &s); SPI_Cmd(SPI2, ENABLE);
}

uint8_t SPI2_Transfer(uint8_t d) {
    while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE)==RESET);
    SPI_I2S_SendData(SPI2, d);
    while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE)==RESET);
    return (uint8_t)SPI_I2S_ReceiveData(SPI2);
}

/* ===== RX 模块 NRF 操作 ===== */
uint8_t rx_rd(uint8_t reg) { RX_CSN_L(); SPI2_Transfer(R_REG|reg); uint8_t v=SPI2_Transfer(NOP); RX_CSN_H(); return v; }
void rx_wr(uint8_t reg, uint8_t v) { RX_CSN_L(); SPI2_Transfer(W_REG|reg); SPI2_Transfer(v); RX_CSN_H(); }
void rx_wrbuf(uint8_t reg, const uint8_t *b, uint8_t n) {
    RX_CSN_L(); SPI2_Transfer(W_REG|reg); while(n--) SPI2_Transfer(*b++); RX_CSN_H();
}

void RX_Init(void) {
    SPI2_Init(); Delay_ms(100);
    rx_wr(CONFIG, 0x0E); delay_us(1500);
    rx_wr(EN_AA, 0x00);
    rx_wr(EN_RXADDR, 0x01);
    rx_wr(SETUP_AW, 0x03);
    rx_wr(RF_CH, ch);
    rx_wr(RF_SETUP, 0x0F);
    rx_wr(RX_PW_P0, 4);
    rx_wrbuf(RX_ADDR_P0, addr, 5);
    rx_wr(CONFIG, 0x0E);
    RX_CSN_L(); SPI2_Transfer(FLUSH_RX); RX_CSN_H();
    RX_CE_H(); Delay_ms(1);
    printf("RX init: CONFIG=0x%02X STATUS=0x%02X\n", rx_rd(CONFIG), rx_rd(STATUS));
}

/* ===== TX 模块 NRF 操作 (SPI1) ===== */
void TX_Init(void) {
    SPI1_Init(); Delay_ms(100);
    NRF_CSN_HIGH(); NRF_CE_LOW();

    // 借用已有的 SPI1 nrf 函数，直接操作寄存器
    auto wr = [](uint8_t r,uint8_t v){ NRF_CSN_LOW(); SPI1_Transfer(W_REG|r); SPI1_Transfer(v); NRF_CSN_HIGH(); };
    auto wrbuf = [](uint8_t r,const uint8_t*b,uint8_t n){ NRF_CSN_LOW(); SPI1_Transfer(W_REG|r); while(n--) SPI1_Transfer(*b++); NRF_CSN_HIGH(); };

    wr(CONFIG, 0x0E); delay_us(1500);
    wr(EN_AA, 0x00);
    wr(EN_RXADDR, 0x01);
    wr(SETUP_AW, 0x03);
    wr(RF_CH, ch);
    wr(RF_SETUP, 0x0F);
    wrbuf(TX_ADDR, addr, 5);
    wrbuf(RX_ADDR_P0, addr, 5);
    wr(RX_PW_P0, 4);
    NRF_CE_H(); Delay_ms(1);

    uint8_t cfg, st;
    NRF_CSN_LOW(); SPI1_Transfer(R_REG|CONFIG); cfg=SPI1_Transfer(NOP); NRF_CSN_H();
    NRF_CSN_LOW(); SPI1_Transfer(R_REG|STATUS); st=SPI1_Transfer(NOP); NRF_CSN_H();
    printf("TX init: CONFIG=0x%02X STATUS=0x%02X\n", cfg, st);
}

void TX_Send(const uint8_t *d) {
    NRF_CSN_LOW(); SPI1_Transfer(W_TX_PL); SPI1_Transfer(d[0]); SPI1_Transfer(d[1]);
    SPI1_Transfer(d[2]); SPI1_Transfer(d[3]); NRF_CSN_H(); Delay_us(20);
}

void RX_Check(void) {
    uint8_t st;
    RX_CSN_LOW(); SPI2_Transfer(R_REG|STATUS); st=SPI2_Transfer(NOP); RX_CSN_H();
    if ((st & 0x0E) != 0x0E) {
        uint8_t b[4];
        RX_CSN_LOW(); SPI2_Transfer(R_RX_PL);
        b[0]=SPI2_Transfer(NOP); b[1]=SPI2_Transfer(NOP);
        b[2]=SPI2_Transfer(NOP); b[3]=SPI2_Transfer(NOP);
        RX_CSN_H();
        printf("RX GOT: %02X %02X %02X %02X", b[0],b[1],b[2],b[3]);
        if (b[0]==0xAA && b[3]==0x55) {
            int16_t r = ((int16_t)b[1]<<8)|b[2];
            printf(" -> TEMP: %.1fC\n", r/10.0f);
        } else {
            printf(" (raw)\n");
        }
    }
}

int main(void)
{
    uint8_t pkt[4]; uint32_t last=0, now; int count=0;
    SystemClock_Config(); Delay_Init();
    USART1_Init(); USART2_Init();

    printf("\r\n=== NRF24L01 TX/RX Test ===\r\n");
    TX_Init(); RX_Init();

    while(1) {
        now = DWT->CYCCNT/(SystemCoreClock/1000);
        if ((now-last)>=1000) {
            last=now;
            pkt[0]=0xAA; pkt[1]=0x12; pkt[2]=0x34; pkt[3]=0x55;
            TX_Send(pkt);
            printf("TX sent #%d: AA 12 34 55\r\n", ++count);
        }
        RX_Check();
        Delay_ms(5);
    }
}

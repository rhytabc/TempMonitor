#include "spi.h"

/**
 * SPI1 初始化：PA5=SCK, PA6=MISO, PA7=MOSI
 * 模式：主模式，8 位数据，CPOL=0, CPHA=0
 * 速率：9MHz（72MHz/8），NRF24L01 最高支持 10MHz
 */
void SPI1_Init(void)
{
    GPIO_InitTypeDef  gpio;
    SPI_InitTypeDef   spi;

    /* 使能时钟 */
    RCC_APB2PeriphClockCmd(SPI1_RCC | NRF_GPIO_RCC | RCC_APB2Periph_SPI1, ENABLE);

    /* SCK (PA5): 复用推挽 */
    gpio.GPIO_Pin   = SPI1_SCK_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SPI1_PORT, &gpio);

    /* MISO (PA6): 浮空输入 */
    gpio.GPIO_Pin   = SPI1_MISO_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(SPI1_PORT, &gpio);

    /* MOSI (PA7): 复用推挽 */
    gpio.GPIO_Pin   = SPI1_MOSI_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(SPI1_PORT, &gpio);

    /* CE (PB1) + CSN (PB0): 推挽输出 */
    gpio.GPIO_Pin   = NRF_CE_PIN | NRF_CSN_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(NRF_CE_PORT, &gpio);

    /* 初始状态 */
    NRF_CSN_HIGH();
    NRF_CE_LOW();

    /* SPI1 配置 */
    SPI_StructInit(&spi);
    spi.SPI_Mode              = SPI_Mode_Master;
    spi.SPI_Direction         = SPI_Direction_2Lines_FullDuplex;
    spi.SPI_DataSize          = SPI_DataSize_8b;
    spi.SPI_CPOL              = SPI_CPOL_Low;
    spi.SPI_CPHA              = SPI_CPHA_1Edge;
    spi.SPI_NSS               = SPI_NSS_Soft;
    spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;  /* 72/8=9MHz */
    spi.SPI_FirstBit          = SPI_FirstBit_MSB;
    SPI_Init(SPI1, &spi);

    SPI_Cmd(SPI1, ENABLE);
}

/**
 * SPI1 收发一个字节（同时发送和接收）
 */
uint8_t SPI1_Transfer(uint8_t data)
{
    /* 等待发送缓冲区空 */
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

    /* 发送数据 */
    SPI_I2S_SendData(SPI1, data);

    /* 等待接收完成 */
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

    /* 返回接收到的数据 */
    return (uint8_t)SPI_I2S_ReceiveData(SPI1);
}

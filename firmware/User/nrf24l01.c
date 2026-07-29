#include "nrf24l01.h"
#include "spi.h"
#include "delay.h"

/**
 * NRF24L01 发送端驱动（PTX 模式）
 * 用于 STM32F103C8T6 通过 SPI1 控制 NRF24L01 发送温度数据
 */

/* 5 字节收发地址 */
static const uint8_t tx_addr[5] = {'T', 'S', 'E', 'N', 'S'};

/*---------- 底层 SPI 操作 ----------*/

static uint8_t NRF_ReadReg(uint8_t reg)
{
    uint8_t val;
    NRF_CSN_LOW();
    SPI1_Transfer(NRF_CMD_R_REGISTER | reg);
    val = SPI1_Transfer(NRF_CMD_NOP);
    NRF_CSN_HIGH();
    return val;
}

static void NRF_WriteReg(uint8_t reg, uint8_t val)
{
    NRF_CSN_LOW();
    SPI1_Transfer(NRF_CMD_W_REGISTER | reg);
    SPI1_Transfer(val);
    NRF_CSN_HIGH();
}

static void NRF_WriteBuf(uint8_t reg, const uint8_t *buf, uint8_t len)
{
    NRF_CSN_LOW();
    SPI1_Transfer(NRF_CMD_W_REGISTER | reg);
    while (len--)
    {
        SPI1_Transfer(*buf++);
    }
    NRF_CSN_HIGH();
}

static void NRF_FlushTX(void)
{
    NRF_CSN_LOW();
    SPI1_Transfer(NRF_CMD_FLUSH_TX);
    NRF_CSN_HIGH();
}

/**
 * @brief 初始化 NRF24L01 为发送模式
 *
 * 配置：
 *   - 通道: 2500MHz
 *   - 速率: 2Mbps
 *   - 功率: 0dBm
 *   - 地址: "TSENS"
 *   - 自动应答: 关闭（单向发送）
 *   - CRC: 1 字节
 */
void NRF24L01_Init(void)
{
    /* 上电后等待稳定（NRF24L01 需要 100ms 上电延时） */
    Delay_ms(100);

    /* CE 拉低进入待机模式 */
    NRF_CE_LOW();

    /* 发送地址（TX_ADDR 和 RX_ADDR_P0 设相同地址） */
    NRF_WriteBuf(NRF_REG_TX_ADDR, tx_addr, 5);
    NRF_WriteBuf(NRF_REG_RX_ADDR_P0, tx_addr, 5);

    /* 接收通道 0 有效载荷宽度 = 4 字节 */
    NRF_WriteReg(NRF_REG_RX_PW_P0, NRF_PAYLOAD_SIZE);

    /* 自动应答: 全部关闭 */
    NRF_WriteReg(NRF_REG_EN_AA, 0x00);

    /* 接收地址: 只启用通道 0 */
    NRF_WriteReg(NRF_REG_EN_RXADDR, 0x01);

    /* 地址宽度: 5 字节 */
    NRF_WriteReg(NRF_REG_SETUP_AW, 0x03);

    /* 自动重发: 关闭 */
    NRF_WriteReg(NRF_REG_SETUP_RETR, 0x00);

    /* 射频通道: 2400 + 100 = 2500 MHz */
    NRF_WriteReg(NRF_REG_RF_CH, NRF_CHANNEL);

    /* 射频配置: 250Kbps, 0dBm */
    NRF_WriteReg(NRF_REG_RF_SETUP, 0x27);

    /* 主配置: TX 模式, CRC 1 字节, 上电 */
    NRF_WriteReg(NRF_REG_CONFIG, 0x0E);  /* PWR_UP=1, PRIM_RX=0, CRC=DISABLED 测试 */

    /* CE 拉高进入发送模式 */
    NRF_CE_HIGH();

    Delay_ms(2);
}

/**
 * @brief 发送一包数据（4 字节）
 * @param data 4 字节数据指针
 */
void NRF24L01_Send(const uint8_t *data)
{
    /* 写 TX FIFO */
    NRF_CSN_LOW();
    SPI1_Transfer(NRF_CMD_W_TX_PAYLOAD);
    SPI1_Transfer(data[0]);
    SPI1_Transfer(data[1]);
    SPI1_Transfer(data[2]);
    SPI1_Transfer(data[3]);
    NRF_CSN_HIGH();

    /* 短暂延时让发送完成 */
    Delay_us(20);
}

/**
 * @brief 检查是否正在发送
 * @return 1=发送中, 0=空闲
 */
uint8_t NRF24L01_IsSending(void)
{
    uint8_t status = NRF_ReadReg(NRF_REG_STATUS);
    return (status & 0x01) ? 1 : 0;  /* TX_FULL = bit 0 */
}

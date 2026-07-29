#ifndef __NRF24L01_H
#define __NRF24L01_H

#include "stm32f10x.h"
#include <stdint.h>

/* NRF24L01 命令 */
#define NRF_CMD_R_REGISTER      0x00
#define NRF_CMD_W_REGISTER      0x20
#define NRF_CMD_R_RX_PAYLOAD    0x61
#define NRF_CMD_W_TX_PAYLOAD    0xA0
#define NRF_CMD_FLUSH_TX        0xE1
#define NRF_CMD_FLUSH_RX        0xE2
#define NRF_CMD_REUSE_TX_PL     0xE3
#define NRF_CMD_NOP             0xFF

/* NRF24L01 寄存器 */
#define NRF_REG_CONFIG          0x00
#define NRF_REG_EN_AA           0x01
#define NRF_REG_EN_RXADDR       0x02
#define NRF_REG_SETUP_AW        0x03
#define NRF_REG_SETUP_RETR      0x04
#define NRF_REG_RF_CH           0x05
#define NRF_REG_RF_SETUP        0x06
#define NRF_REG_STATUS          0x07
#define NRF_REG_TX_ADDR         0x10
#define NRF_REG_RX_ADDR_P0      0x0A
#define NRF_REG_RX_ADDR_P1      0x0B
#define NRF_REG_RX_PW_P0        0x11
#define NRF_REG_FIFO_STATUS     0x17

/* 射频通道（2400 + CHANNEL MHz） */
#define NRF_CHANNEL             100

/* 数据包宽度 */
#define NRF_PAYLOAD_SIZE        4

void NRF24L01_Init(void);
void NRF24L01_Send(const uint8_t *data);
uint8_t NRF24L01_IsSending(void);

#endif

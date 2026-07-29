#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"
#include <stdint.h>
#include <stdio.h>

/**
 * USART1 串口通信模块
 *
 * 通过 PA9（TX）/ PA10（RX）与上位机（PC / 手机等）进行有线通信。
 * 数据格式：115200bps，8 个数据位，无校验，1 个停止位（8N1）。
 *
 * 硬件连接：
 *   PA9  → USB转TTL模块的 RXD（接收）
 *   PA10 → USB转TTL模块的 TXD（发送）
 *   GND  → USB转TTL模块的 GND（共地）
 *
 * 注意：这是目前的"有线"方案，后续可替换为蓝牙/WiFi无线模块。
 */

#define USART_BAUDRATE      115200  /* 降低波特率，增强容错 */

/*---------- 基本收发函数 ----------*/
void    USART1_Init(void);
void    USART1_SendByte(uint8_t data);
void    USART1_SendString(const char *str);
void    USART1_SendBuf(const uint8_t *buf, uint16_t len);
uint8_t USART1_IsDataReady(void);
uint8_t USART1_ReadByte(void);

/* USART2 (PA2=TX, 备用) */
void    USART2_Init(void);
void    USART2_SendByte(uint8_t data);
void    USART2_SendString(const char *str);

int fputc(int ch, FILE *f);

#endif

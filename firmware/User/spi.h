#ifndef __SPI_H
#define __SPI_H

#include "stm32f10x.h"
#include <stdint.h>

/* SPI1 引脚定义 */
#define SPI1_SCK_PIN    GPIO_Pin_5   /* PA5 */
#define SPI1_MISO_PIN   GPIO_Pin_6   /* PA6 */
#define SPI1_MOSI_PIN   GPIO_Pin_7   /* PA7 */
#define SPI1_PORT       GPIOA
#define SPI1_RCC        RCC_APB2Periph_GPIOA

/* NRF24L01 SPI 口线 */
#define NRF_CE_PORT     GPIOB
#define NRF_CE_PIN      GPIO_Pin_1   /* PB1 */
#define NRF_CSN_PORT    GPIOB
#define NRF_CSN_PIN     GPIO_Pin_0   /* PB0 */
#define NRF_GPIO_RCC    RCC_APB2Periph_GPIOB

/* 片选宏 */
#define NRF_CSN_LOW()   GPIO_ResetBits(NRF_CSN_PORT, NRF_CSN_PIN)
#define NRF_CSN_HIGH()  GPIO_SetBits(NRF_CSN_PORT, NRF_CSN_PIN)
#define NRF_CE_LOW()    GPIO_ResetBits(NRF_CE_PORT, NRF_CE_PIN)
#define NRF_CE_HIGH()   GPIO_SetBits(NRF_CE_PORT, NRF_CE_PIN)

void SPI1_Init(void);
uint8_t SPI1_Transfer(uint8_t data);

#endif

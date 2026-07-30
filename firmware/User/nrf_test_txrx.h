#ifndef __NRF_TEST_TXRX_H
#define __NRF_TEST_TXRX_H

#include "stm32f10x.h"
#include <stdint.h>

/* ===== TX 模块引脚 (SPI1) ===== */
#define TX_CE_PORT   GPIOB
#define TX_CE_PIN    GPIO_Pin_1   /* PB1 */
#define TX_CSN_PORT  GPIOB
#define TX_CSN_PIN   GPIO_Pin_0   /* PB0 */

/* ===== RX 模块引脚 (SPI2) ===== */
#define RX_CE_PORT   GPIOB
#define RX_CE_PIN    GPIO_Pin_11  /* PB11 */
#define RX_CSN_PORT  GPIOB
#define RX_CSN_PIN   GPIO_Pin_10  /* PB10 */

/* SPI2 引脚 (PB13=SCK, PB14=MISO, PB15=MOSI) */

void  NRF_Test_Init(void);
void  NRF_Test_TX_Send(const uint8_t *data);
void  NRF_Test_RX_Loop(void);  /* RX 主循环，收到数据就 printf */

#endif

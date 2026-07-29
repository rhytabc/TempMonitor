#ifndef __ONEWIRE_H
#define __ONEWIRE_H

#include "stm32f10x.h"
#include <stdint.h>

/**
 * 1-Wire 单总线驱动模块
 *
 * 使用 PB12 作为数据引脚（开漏输出 + 外部 4.7kΩ 上拉电阻）。
 * 遵循 Maxim/Dallas 1-Wire 标准时序。
 *
 * 硬件连接：
 *   PB12  →  DS18B20 的 DQ 引脚
 *   同时需要在 DQ 和 3.3V 之间接一个 4.7kΩ 上拉电阻
 */

/*---------- 引脚定义 ----------*/
#define OW_PORT         GPIOB              /* 数据端口：GPIOB */
#define OW_PIN          GPIO_Pin_12        /* 数据引脚：PB12 */
#define OW_RCC          RCC_APB2Periph_GPIOB  /* 端口时钟 */

/*---------- 引脚操作宏 ----------*/
#define OW_LOW()        GPIO_ResetBits(OW_PORT, OW_PIN)   /* 拉低总线 */
#define OW_HIGH()       GPIO_SetBits(OW_PORT, OW_PIN)     /* 释放总线（上拉电阻拉高） */
#define OW_READ()       (GPIO_ReadInputDataBit(OW_PORT, OW_PIN))  /* 读取总线电平 */

/*---------- 函数声明 ----------*/
void     OW_Init(void);               /* 初始化单总线引脚 */
uint8_t  OW_Reset(void);              /* 发送复位脉冲，检测设备是否存在（返回0=存在） */
void     OW_WriteByte(uint8_t data);  /* 向总线写入一个字节（LSB 优先） */
uint8_t  OW_ReadByte(void);           /* 从总线读取一个字节（LSB 优先） */

#endif

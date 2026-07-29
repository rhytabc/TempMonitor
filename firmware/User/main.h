#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f10x.h"

/**
 * 主程序头文件
 *
 * 项目：DS18B20 温度监控器
 * 芯片：STM32F103C8T6
 * 功能：读取 DS18B20 温度传感器数据，通过 USART1 串口发送到上位机
 */

/*---------- 板载 LED 定义 ----------*/
#define LED_PORT    GPIOC              /* LED 端口：GPIOC */
#define LED_PIN     GPIO_Pin_13        /* LED 引脚：PC13（低电平点亮） */
#define LED_RCC     RCC_APB2Periph_GPIOC  /* LED 端口时钟 */

/*---------- 函数声明 ----------*/
void SystemClock_Config(void);         /* 配置系统时钟（HSE 8MHz → PLL 72MHz） */
void LED_Init(void);                   /* 初始化板载 LED（PC13） */

#endif

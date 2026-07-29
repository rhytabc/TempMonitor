/**
 * DS18B20 温度监控器
 * STM32F103C8T6 — USART2(PA2) 输出温度数据给 ESP32
 * 波特率: 9600
 */
#include "main.h"
#include "delay.h"
#include "onewire.h"
#include "ds18b20.h"
#include "usart.h"
#include <stdio.h>

#define SEND_MS 1000

void SystemClock_Config(void) {
    ErrorStatus s; RCC_DeInit(); RCC_HSEConfig(RCC_HSE_ON);
    s = RCC_WaitForHSEStartUp(); if (s == ERROR) while (1);
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); RCC_PLLCmd(ENABLE);
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
    FLASH_SetLatency(FLASH_Latency_2);
    RCC_HCLKConfig(RCC_SYSCLK_Div1); RCC_PCLK2Config(RCC_HCLK_Div1);
    RCC_PCLK1Config(RCC_HCLK_Div2); RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    while (RCC_GetSYSCLKSource() != 0x08); SystemCoreClock = 72000000;
}

int main(void) {
    float temp; uint32_t last = 0, now;
    SystemClock_Config(); Delay_Init();
    GPIO_InitTypeDef g; RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
    g.GPIO_Pin=GPIO_Pin_13;g.GPIO_Mode=GPIO_Mode_Out_PP;g.GPIO_Speed=GPIO_Speed_2MHz;
    GPIO_Init(GPIOC,&g);GPIO_SetBits(GPIOC,GPIO_Pin_13);
    USART1_Init(); USART2_Init(); OW_Init();

    printf("\r\n=== DS18B20 Temp Monitor ===\r\n");
    if (DS18B20_Init() != 0) printf("WARN: Sensor not found\r\n");

    while (1) {
        now = DWT->CYCCNT / (SystemCoreClock / 1000);
        if ((now - last) >= SEND_MS) {
            last = now;
            if (DS18B20_ReadTemp(&temp) == 0)
                printf("T:%.1f\r\n", temp), GPIO_WriteBit(GPIOC,GPIO_Pin_13,(BitAction)(1-GPIO_ReadOutputDataBit(GPIOC,GPIO_Pin_13)));
        }
    }
}

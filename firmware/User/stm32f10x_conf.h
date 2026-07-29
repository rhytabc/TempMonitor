#ifndef __STM32F10x_CONF_H
#define __STM32F10x_CONF_H

/* 禁用 assert_param（Keil ARMCLANG v6 兼容） */
#define assert_param(expr) ((void)0)

/* 只启用我们需要的模块，注释掉不需要的以减小体积 */
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_flash.h"
#include "stm32f10x_spi.h"
#include "misc.h"

#endif

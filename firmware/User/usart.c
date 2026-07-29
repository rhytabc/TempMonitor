#include "usart.h"

/* ==================== USART1 ==================== */

void USART1_Init(void)
{
    GPIO_InitTypeDef  gpio;
    USART_InitTypeDef usart;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    gpio.GPIO_Pin   = GPIO_Pin_9;
    gpio.GPIO_Mode  = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    gpio.GPIO_Pin   = GPIO_Pin_10;
    gpio.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);

    USART_StructInit(&usart);
    usart.USART_BaudRate            = USART_BAUDRATE;
    usart.USART_WordLength          = USART_WordLength_8b;
    usart.USART_StopBits            = USART_StopBits_1;
    usart.USART_Parity              = USART_Parity_No;
    usart.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &usart);
    USART_Cmd(USART1, ENABLE);
}

void USART1_SendByte(uint8_t data)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, data);
}

/* ==================== USART2 (PA2=TX, PA3=RX) ==================== */

void USART2_Init(void)
{
    GPIO_InitTypeDef  gpio;
    USART_InitTypeDef usart;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);   // PA2
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);   // APB1

    gpio.GPIO_Pin   = GPIO_Pin_2;          // PA2 = TX
    gpio.GPIO_Mode  = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    gpio.GPIO_Pin   = GPIO_Pin_3;          // PA3 = RX
    gpio.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);

    USART_StructInit(&usart);
    usart.USART_BaudRate            = USART_BAUDRATE;
    usart.USART_WordLength          = USART_WordLength_8b;
    usart.USART_StopBits            = USART_StopBits_1;
    usart.USART_Parity              = USART_Parity_No;
    usart.USART_Mode                = USART_Mode_Tx;  // 只发不收
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART2, &usart);
    USART_Cmd(USART2, ENABLE);
}

void USART2_SendByte(uint8_t data)
{
    while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    USART_SendData(USART2, data);
}

void USART2_SendString(const char *str)
{
    while (*str) USART2_SendByte((uint8_t)*str++);
}

/* ==================== printf 重定向 → USART1 + USART2 ==================== */

#if defined(__MICROLIB)
int fputc(int ch, FILE *f)
{
    (void)f;
    USART1_SendByte((uint8_t)ch);  // 原串口保留
    USART2_SendByte((uint8_t)ch);  // 同时发到 USART2
    return ch;
}
#else
int __io_putchar(int ch)
{
    USART1_SendByte((uint8_t)ch);
    USART2_SendByte((uint8_t)ch);
    return ch;
}
#endif

/**
 * 1-Wire 单总线驱动模块
 *
 * 通过 PB12 引脚（开漏模式）与 DS18B20 通信。
 * 电气要求：DQ 引脚必须外接 4.7kΩ 上拉电阻到 3.3V。
 *
 * DS18B20 时序要求（标准速度模式）：
 *   写1时序：  主机拉低 1~15us，然后在 15~60us 内采样，总共 >60us
 *   写0时序：  主机拉低 60~120us，总共 >60us
 *   读时序：   主机拉低 1~15us，在 15us 内采样，总共 >60us
 *   复位脉冲： 主机拉低 480~960us，设备应答 60~240us 的低脉冲
 *
 * 注意：操作期间必须关中断，否则时序会被打断导致通信失败。
 */

#include "onewire.h"
#include "delay.h"

/**
 * @brief 初始化单总线引脚
 *
 * 配置 PB12 为开漏输出模式。
 * 开漏输出 + 外部上拉电阻是 1-Wire 总线的标准接法，
 * 允许多个设备共享同一条数据线。
 */
void OW_Init(void)
{
    GPIO_InitTypeDef gpio;

    /* 使能 GPIOB 时钟 */
    RCC_APB2PeriphClockCmd(OW_RCC, ENABLE);

    /* 配置 PB12：开漏输出，50MHz */
    gpio.GPIO_Pin   = OW_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_Out_OD;   /* 开漏输出 —— 1-Wire 总线必需 */
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(OW_PORT, &gpio);

    /* 释放总线（外部上拉电阻将其拉高，表示空闲状态） */
    OW_HIGH();
}

/**
 * @brief 发送复位脉冲，检测总线是否有设备
 * @return 0 = 检测到设备，1 = 无设备
 *
 * 复位流程：
 *   1. 主机拉低总线 480us 以上
 *   2. 主机释放总线
 *   3. 如果总线上有 DS18B20，它会在 60~240us 内拉低总线作为应答
 *   4. 主机在释放后约 70us 采样总线电平
 */
uint8_t OW_Reset(void)
{
    uint8_t presence;

    /* 第1步：拉低总线 500us（复位脉冲，要求 ≥480us） */
    OW_LOW();
    Delay_us(500);

    /* 第2步：释放总线，等待 70us */
    OW_HIGH();
    Delay_us(70);

    /* 第3步：采样总线 — 设备拉低 = 存在，保持高 = 不存在 */
    presence = OW_READ();

    /* 第4步：等待复位时序结束（总共约 960us） */
    Delay_us(420);

    return presence;  /* 0 = 有设备，1 = 无设备 */
}

/**
 * @brief 向总线写入一个位
 * @param bit 要写入的位（0 或 1）
 */
static void OW_WriteBit(uint8_t bit)
{
    if (bit)
    {
        /*--- 写 1：短暂拉低后释放 ---*/
        OW_LOW();
        Delay_us(2);       /* 拉低 2us */
        OW_HIGH();
        Delay_us(60);      /* 等待时序完成（总共 >60us） */
    }
    else
    {
        /*--- 写 0：拉低 60us ---*/
        OW_LOW();
        Delay_us(65);      /* 拉低 65us（要求 60~120us） */
        OW_HIGH();
        Delay_us(5);       /* 恢复时间 */
    }
}

/**
 * @brief 从总线读取一个位
 * @return 读取到的位值（0 或 1）
 */
static uint8_t OW_ReadBit(void)
{
    uint8_t bit;

    /* 第1步：主机拉低总线 2us，发起读时序 */
    OW_LOW();
    Delay_us(2);

    /* 第2步：释放总线，延时 1us 后立即采样 */
    OW_HIGH();
    Delay_us(1);

    bit = OW_READ();  /* 在 15us 内采样 */

    /* 第3步：等待读时序完成（总共 >60us） */
    Delay_us(60);

    return bit;
}

/**
 * @brief 向总线写入一个字节（LSB 优先，即低位在前）
 * @param data 要写入的字节
 */
void OW_WriteByte(uint8_t data)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        OW_WriteBit(data & 0x01);  /* 发送最低位 */
        data >>= 1;                 /* 右移，准备发送下一位 */
    }
}

/**
 * @brief 从总线读取一个字节（LSB 优先，即低位在前）
 * @return 读取到的字节
 */
uint8_t OW_ReadByte(void)
{
    uint8_t i, data = 0;
    for (i = 0; i < 8; i++)
    {
        if (OW_ReadBit())
        {
            data |= (1 << i);  /* 读取到的位放到对应位置 */
        }
    }
    return data;
}

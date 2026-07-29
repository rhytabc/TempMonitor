/**
 * DS18B20 数字温度传感器驱动模块
 *
 * 本驱动假定总线上只有一个 DS18B20 设备，使用 SKIP_ROM 指令简化通信。
 * 如果总线上有多个传感器，需要改用 MATCH_ROM + 64 位 ROM 编码来寻址。
 *
 * 温度转换流程（单次读取）：
 *   1. 复位 → 发 SKIP_ROM → 发 CONVERT_T（启动转换）
 *   2. 等待转换完成（12位精度需 750ms）
 *   3. 复位 → 发 SKIP_ROM → 发 READ_SCRATCH（读取暂存器）
 *   4. 从暂存器的前2个字节解析温度值
 *   5. CRC 校验（确保数据完整性）
 */

#include "ds18b20.h"
#include "onewire.h"
#include "delay.h"

/**
 * @brief 初始化 / 检测传感器
 * @return 0 = 传感器存在，1 = 未检测到传感器
 *
 * 通过发送复位脉冲来检测总线上是否有 DS18B20。
 * 如果返回 1，请检查：
 *   1. 接线是否正确（VDD、GND、DQ）
 *   2. DQ 引脚是否接了 4.7kΩ 上拉电阻
 *   3. 传感器是否完好
 */
uint8_t DS18B20_Init(void)
{
    return OW_Reset();  /* OW_Reset 返回 0 表示有设备应答 */
}

/**
 * @brief 启动温度转换（不等待完成）
 *
 * 发送 SKIP_ROM（跳过地址匹配）+ CONVERT_T（开始转换）命令。
 * DS18B20 收到后会开始内部 ADC 转换，12 位精度最大需要 750ms。
 * 此函数不等待转换完成，适合需要做其他事情的场景。
 */
static void DS18B20_StartConversion(void)
{
    OW_Reset();                          /* 复位总线 */
    OW_WriteByte(DS18B20_CMD_SKIP_ROM);  /* 跳过 ROM 匹配（单设备模式） */
    OW_WriteByte(DS18B20_CMD_CONVERT_T); /* 启动温度转换 */
}

/**
 * @brief 读取暂存器并解析温度值
 * @param temp 输出参数，存放解析后的温度值（单位：°C）
 * @return 0 = 成功，1 = CRC 校验失败
 *
 * DS18B20 暂存器（共 9 字节）结构：
 *   字节 0    : 温度值低字节（LSB）
 *   字节 1    : 温度值高字节（MSB）
 *   字节 2    : 高温报警阈值（TH）
 *   字节 3    : 低温报警阈值（TL）
 *   字节 4    : 配置寄存器（分辨率设置）
 *   字节 5~7  : 保留
 *   字节 8    : 前 8 字节的 CRC 校验值
 *
 * 温度数据格式（12 位）：
 *   高字节的 bit7~bit3 是符号位扩展（全0=正温，全1=负温）
 *   温度 = 原始值 / 16（即每个 LSB = 0.0625°C）
 */
static uint8_t DS18B20_ReadScratchpad(float *temp)
{
    uint8_t  buf[9];
    uint8_t  i;
    int16_t  raw;

    /* 第1步：发送读暂存器命令 */
    OW_Reset();
    OW_WriteByte(DS18B20_CMD_SKIP_ROM);
    OW_WriteByte(DS18B20_CMD_READ_SCRATCH);

    /* 第2步：读取 9 字节暂存器数据 */
    for (i = 0; i < 9; i++)
    {
        buf[i] = OW_ReadByte();
    }

    /* 第3步：CRC8 校验（Maxim/Dallas 1-Wire 标准 CRC） */
    {
        uint8_t crc = 0;
        for (i = 0; i < 8; i++)        /* 只校验前 8 字节 */
        {
            uint8_t byte = buf[i];
            uint8_t j;
            for (j = 0; j < 8; j++)
            {
                uint8_t mix = (crc ^ byte) & 0x01;
                crc >>= 1;
                if (mix) crc ^= 0x8C;  /* CRC-8/Dallas 多项式 */
                byte >>= 1;
            }
        }
        if (crc != buf[8])
        {
            return 1;  /* CRC 不匹配，数据可能出错 */
        }
    }

    /* 第4步：组合温度原始值（低字节 | 高字节<<8） */
    raw = (int16_t)(buf[1] << 8) | buf[0];

    /* 第5步：换算为摄氏度（原始值 / 16） */
    *temp = (float)raw / 16.0f;

    return 0;
}

/**
 * @brief 执行一次完整的温度读取（阻塞式，约 800ms）
 * @param temp 输出参数，存放温度值（单位：°C）
 * @return 0 = 成功，1 = 失败
 *
 * 完整流程：启动转换 → 等待 800ms → 读取暂存器 → CRC 校验
 * 失败时自动重试最多 3 次。
 *
 * 注意：此函数会阻塞约 800ms，因为必须等待 DS18B20 完成内部转换。
 * 如果对实时性有要求，可以改用异步方式（StartConversion + 定时器 + ReadScratchpad）。
 */
uint8_t DS18B20_ReadTemp(float *temp)
{
    uint8_t retry = 3;  /* 最多重试 3 次 */

    /* 启动温度转换 */
    DS18B20_StartConversion();

    /* 等待转换完成（12 位分辨率最大 750ms，这里取 800ms 留余量） */
    Delay_ms(800);

    /* 读取并校验，失败则重试 */
    while (retry--)
    {
        if (DS18B20_ReadScratchpad(temp) == 0)
        {
            return 0;  /* 读取成功 */
        }
        Delay_ms(10);  /* 重试前稍作等待 */
    }

    return 1;  /* 多次重试均失败 */
}

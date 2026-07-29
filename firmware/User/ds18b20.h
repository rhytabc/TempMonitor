#ifndef __DS18B20_H
#define __DS18B20_H

#include "stm32f10x.h"
#include <stdint.h>

/**
 * DS18B20 数字温度传感器驱动模块
 *
 * DS18B20 是 Maxim（美信）公司生产的数字温度传感器，
 * 使用 1-Wire 单总线通信协议，只需一根数据线 + 电源 + 地线。
 *
 * 主要特性：
 *   - 测温范围：-55°C ~ +125°C
 *   - 分辨率：12 位（0.0625°C / LSB）
 *   - 精度：±0.5°C（-10°C ~ +85°C 范围内）
 *   - 转换时间：最大 750ms（12 位分辨率）
 *
 * 注意：DS18B20 是数字传感器，输出的是数字信号，不需要 AD 采样。
 */

/*---------- DS18B20 ROM 操作命令 ----------*/
#define DS18B20_CMD_SEARCH_ROM      0xF0  /* 搜索 ROM（多设备时用） */
#define DS18B20_CMD_READ_ROM        0x33  /* 读取 ROM 编码（仅单设备时用） */
#define DS18B20_CMD_MATCH_ROM       0x55  /* 匹配 ROM（选中指定设备） */
#define DS18B20_CMD_SKIP_ROM        0xCC  /* 跳过 ROM（单设备时用这个最方便） */
#define DS18B20_CMD_ALARM_SEARCH    0xEC  /* 报警搜索 */

/*---------- DS18B20 功能操作命令 ----------*/
#define DS18B20_CMD_CONVERT_T       0x44  /* 启动温度转换 */
#define DS18B20_CMD_WRITE_SCRATCH   0x4E  /* 写暂存器（配置分辨率等） */
#define DS18B20_CMD_READ_SCRATCH    0xBE  /* 读暂存器（读取温度值） */
#define DS18B20_CMD_COPY_SCRATCH    0x48  /* 将暂存器复制到 EEPROM */
#define DS18B20_CMD_RECALL_E2       0xB8  /* 从 EEPROM 回读配置 */
#define DS18B20_CMD_READ_POWER      0xB4  /* 读取供电模式 */

/*---------- 函数声明 ----------*/
uint8_t DS18B20_Init(void);                    /* 初始化/检测传感器（返回0=成功） */
uint8_t DS18B20_ReadTemp(float *temp);         /* 读取温度值（阻塞，约 800ms） */

#endif

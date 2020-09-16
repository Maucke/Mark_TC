#ifndef __DS3231_H_
#define __DS3231_H_

#include "sys.h"

extern u8 DS3231_Init_Buf[7];
extern u8 DS3231_Us_Buf[7];
#define E2Length 0x30

extern u8 EEPROM_Buf[E2Length];
#define AddrDIM 0x0000
#define AddrTmMt 0x0001
#define AddrMiMt 0x0002
#define AddrBlnk 0x0003
#define AddrEco  0x0004

#define AddrSWALM 0x0005
#define AddrSWECO 0x0006

#define AddrALMH 0x0007
#define AddrALMM 0x0008
#define AddrECOH 0x0009
#define AddrECOM 0x000A
/********************************************************************************************************
** 	DS3231常数定义
********************************************************************************************************/
#define DS3231_WriteAddress 0xD0    //器件写地址 
#define DS3231_ReadAddress  0xD1    //器件读地址
#define DS3231_SECOND       0x00    //秒
#define DS3231_MINUTE       0x01    //分
#define DS3231_HOUR         0x02    //时
#define DS3231_WEEK         0x03    //星期
#define DS3231_DAY          0x04    //日
#define DS3231_MONTH        0x05    //月
#define DS3231_YEAR         0x06    //年
//闹铃1            
#define DS3231_ALARM1SECOND 0x07    //秒
#define DS3231_ALARM1MINUTE 0x08    //分
#define DS3231_ALARM1HOUR   0x09    //时
#define DS3231_ALARM1WEEK   0x0A    //星期/日
//闹铃2
#define DS3231_ALARM2MINUTE 0x0b    //分
#define DS3231_ALARM2HOUR   0x0c    //时
#define DS3231_ALARM2WEEK   0x0d    //星期/日

#define DS3231_CONTROL      0x0e    //控制寄存器
#define DS3231_STATUS       0x0f    //状态寄存器
#define BSY                 2       //忙
#define OSF                 7       //振荡器停止标志
#define DS3231_XTAL         0x10    //晶体老化寄存器
#define DS3231_TEMPERATUREH 0x11    //温度寄存器高字节(8位)
#define DS3231_TEMPERATUREL 0x12    //温度寄存器低字节(高2位)  

extern u8 FlagAlarm;
extern u8 FlagTimeGo;
extern u16 RestCount;

u8	BCD2HEX(u8 val);
u8	HEX2BCD(u8 val);
u8 DS3231_ReadByte(void);
u8 DS3231_WriteData(u8 addr, u8 write_data);
u8 DS3231_Current(void);
u8 DS3231_ReadData(u8 random_addr);
u8 Get_Week(u8 year,u8 month,u8 day);	//计算星期几
void Start_I2C(void);
void Stop_I2C(void);
void DS3231_WriteByte(u8 c);
void DS3231_Ack(bit a);
void DS3231_SetTime(u8 *p);
void DS3231_GetTime(void);
void DS3231_Init(void);
void DS3231_SetUart(void);
void DS3231_AlarmReset(void);
void DS3231_AlarmSet(u8 Type,SW,Hour,Minute);
void DS3231_SetTimePrepar(u8 Type);

#endif
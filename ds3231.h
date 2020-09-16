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
** 	DS3231��������
********************************************************************************************************/
#define DS3231_WriteAddress 0xD0    //����д��ַ 
#define DS3231_ReadAddress  0xD1    //��������ַ
#define DS3231_SECOND       0x00    //��
#define DS3231_MINUTE       0x01    //��
#define DS3231_HOUR         0x02    //ʱ
#define DS3231_WEEK         0x03    //����
#define DS3231_DAY          0x04    //��
#define DS3231_MONTH        0x05    //��
#define DS3231_YEAR         0x06    //��
//����1            
#define DS3231_ALARM1SECOND 0x07    //��
#define DS3231_ALARM1MINUTE 0x08    //��
#define DS3231_ALARM1HOUR   0x09    //ʱ
#define DS3231_ALARM1WEEK   0x0A    //����/��
//����2
#define DS3231_ALARM2MINUTE 0x0b    //��
#define DS3231_ALARM2HOUR   0x0c    //ʱ
#define DS3231_ALARM2WEEK   0x0d    //����/��

#define DS3231_CONTROL      0x0e    //���ƼĴ���
#define DS3231_STATUS       0x0f    //״̬�Ĵ���
#define BSY                 2       //æ
#define OSF                 7       //����ֹͣ��־
#define DS3231_XTAL         0x10    //�����ϻ��Ĵ���
#define DS3231_TEMPERATUREH 0x11    //�¶ȼĴ������ֽ�(8λ)
#define DS3231_TEMPERATUREL 0x12    //�¶ȼĴ������ֽ�(��2λ)  

extern u8 FlagAlarm;
extern u8 FlagTimeGo;
extern u16 RestCount;

u8	BCD2HEX(u8 val);
u8	HEX2BCD(u8 val);
u8 DS3231_ReadByte(void);
u8 DS3231_WriteData(u8 addr, u8 write_data);
u8 DS3231_Current(void);
u8 DS3231_ReadData(u8 random_addr);
u8 Get_Week(u8 year,u8 month,u8 day);	//�������ڼ�
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
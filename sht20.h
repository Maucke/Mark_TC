#ifndef _SHT20_H__
#define _SHT20_H__

#include "sys.h"
#include "delay.h"

//ʹ��P0����Ҫ����������
sbit I2C_SDA = P2^1;
sbit I2C_SCL = P2^0;

#define SHT20_SDA_H    I2C_SDA = 1
#define SHT20_SDA_L    I2C_SDA = 0

#define SHT20_SCL_H    I2C_SCL = 1
#define SHT20_SCL_L    I2C_SCL = 0

#define I2C_ADR_W    0x80
#define I2C_ADR_R    0x81

#define I2C_ACK          0
#define I2C_NACK         1

#define USER_REG_W       0xe6      //д�û��Ĵ�����ַ
#define USER_REG_R       0xe7      //���û��Ĵ�����ַ
#define RESOLUTION_00    0x02      //RH 12bit,T 14bit
#define RESOLUTION_01    0x03      //RH 8bit,T 12bit
#define RESOLUTION_10    0x82      //RH 10bit,T 13bit
#define RESOLUTION_11    0x83      //RH 11bit,T 11bit
#define HOST_START_T     0xe3      //������ʼ�¶Ȳ�������
#define HOST_START_RH    0xe5      //������ʼʪ�Ȳ�������
#define START_T          0xf3      //��������ʼ�¶Ȳ�������
#define START_RH         0xf5      //��������ʼʪ�Ȳ�������
#define SOFT_RESET       0xfe      //�����λ
#define RES_MASK         0x81      //�����־

#define CRC_CHECK        0x131

extern float SHT20_Us_Buf[2];

void SHT20_Start(void);
void SHT20_Stop(void);
void SHT20_Ack(void);
void SHT20_NAck(void);
u8 SHT20_WriteByte(u8 byte);
u8 SHT20_ReadByte(u8 ack);
u8 SHT20_SoftReset(void);
u8 SHT20_SetResolution(void);
u8 SHT20_CheckCrc(u8 *pdat, u8 num, u8 checksum);
void InitSHT20(void);
void SHT20_HostReadValue(void);
void SHT20_ReadValue(void);

#endif

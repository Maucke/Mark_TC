#include "sht20.h"

float SHT20_Us_Buf[2];

u8 SHT20_CheckCrc(u8 *pdat, u8 num, u8 checksum)
{
	u8 crc = 0;	
	char i,j;
	
	for (i = (num-1); i >= 0; --i)
	{ 
		crc ^= (*(pdat+i));
		for (j = 8; j > 0; --j)
		{ 
			if (crc & 0x80) 
				crc = (crc << 1) ^ CRC_CHECK;
			else 
				crc = (crc << 1);
		}
	}
	if (crc != checksum) 
		return 0;
	else 
		return 1;
	
	return 0;
}
void SHT20_HostReadValue(void)
{
	u8 Checksum = 0;
	u16 temp = 0;
	SHT20_Start();
	if (SHT20_WriteByte(I2C_ADR_W) == I2C_ACK)
	{
		if (SHT20_WriteByte(HOST_START_T) == I2C_ACK)
		{
			SHT20_Start();
			if (SHT20_WriteByte(I2C_ADR_R) == I2C_ACK)
			{
				delay_ms(100);
				temp = SHT20_ReadByte(I2C_ACK);
				temp <<= 8;
				temp |= SHT20_ReadByte(I2C_ACK);
//				Checksum = SHT20_ReadByte(I2C_NACK);
//				if (SHT20_CheckCrc((u8 *)(&temp),2,Checksum))
//				{
					temp >>= 2;
					SHT20_Us_Buf[1] = -46.85 + 0.0107251 * temp;
//				}
//				else
//				{
//					flag = 0;
//					InitSHT20();
//				}
			}
		}
	}
	SHT20_Stop();
	SHT20_Start();
	if (SHT20_WriteByte(I2C_ADR_W) == I2C_ACK)
	{
		if (SHT20_WriteByte(HOST_START_RH) == I2C_ACK)
		{
			SHT20_Start();
			if (SHT20_WriteByte(I2C_ADR_R) == I2C_ACK)
			{
				delay_ms(100);
				temp = SHT20_ReadByte(I2C_ACK);
				temp <<= 8;
				temp |= SHT20_ReadByte(I2C_ACK);
//				Checksum = SHT20_ReadByte(I2C_NACK);
//				if (SHT20_CheckCrc((u8 *)(&temp),2,Checksum))
//				{
					temp >>= 2;
					SHT20_Us_Buf[0] = -6 + 0.0076294 * temp;
//				}
//				else
//				{
//					flag = 0;
//					InitSHT20();
//				}
			}
		}
	}
	SHT20_Stop();
}


void SHT20_ReadValue(void)
{
  static u8 SHT20_ChannelControl=0;
  static u8 SHT20_ChannelFlag=0;
  static u8 SHT20_ChannelCount=0;
	u8 Addr;
	u8 Checksum = 0;
	u16 temp = 0;
	switch(SHT20_ChannelControl)
	{
		case 0:Addr = HOST_START_RH;break;
		case 1:Addr = HOST_START_T;break;
	}
	if(!SHT20_ChannelFlag)
	{
		SHT20_Start();
		if (SHT20_WriteByte(I2C_ADR_W) == I2C_ACK)
		{
			if (SHT20_WriteByte(Addr) == I2C_ACK)
			{
				SHT20_Start();
				if (SHT20_WriteByte(I2C_ADR_R) == I2C_ACK)
				{
					SHT20_ChannelFlag = True;
				}
			}
		}
		SHT20_Stop();
	}
	else
	{
		if(SHT20_ChannelCount++==1)
		{
			temp = SHT20_ReadByte(I2C_ACK);
			temp <<= 8;
			temp |= SHT20_ReadByte(I2C_ACK);
//			Checksum = SHT20_ReadByte(I2C_NACK);
//			if (SHT20_CheckCrc((u8 *)(&temp),2,Checksum))
//			{
				switch(SHT20_ChannelControl)
				{
					case 0:
						temp >>= 2;
						SHT20_Us_Buf[0] = -6 + 0.0076294 * temp;break;
					case 1:
						temp >>= 2;
						SHT20_Us_Buf[1] = -46.85 + 0.0107251 * temp;break;
				}
//			}
//			else
//				InitSHT20();
			SHT20_ChannelCount = 0;
			SHT20_ChannelControl =(~SHT20_ChannelControl)&1;
			SHT20_ChannelFlag = False;
		}
	}
}

void InitSHT20(void)
{
	SHT20_SoftReset();
	delay_ms(30);
	SHT20_SetResolution();
}

u8 SHT20_SetResolution(void)                    
{
	u8 flag = 0;
	u8 regvalue = 0;
	SHT20_Start();
	if (SHT20_WriteByte(I2C_ADR_W) == I2C_ACK)
	{
		if (SHT20_WriteByte(USER_REG_R) == I2C_ACK)
		{
			SHT20_Start();
			if (SHT20_WriteByte(I2C_ADR_R) == I2C_ACK)
			{
				regvalue = SHT20_ReadByte(I2C_NACK);
			}
		}
	}
	SHT20_Stop();
	
	regvalue = (regvalue & ~RES_MASK) | RESOLUTION_00;
	
	SHT20_Start();
	if (SHT20_WriteByte(I2C_ADR_W) == I2C_ACK)
	{
		if (SHT20_WriteByte(USER_REG_W) == I2C_ACK)
		{
			if (SHT20_WriteByte(regvalue) == I2C_ACK)
			{
				flag = 1;
			}
		}
	}
	SHT20_Stop();
	return flag;
}

u8 SHT20_SoftReset()
{
	u8 flag = 0;
	SHT20_Start();
	if (SHT20_WriteByte(I2C_ADR_W) == I2C_ACK)
	{
		if (SHT20_WriteByte(SOFT_RESET) == I2C_ACK)
		{
			flag = 1;
		}
	}
	SHT20_Stop();
	return flag;
}

u8 SHT20_ReadByte(u8 ack)
{
	u8 i,Rec = 0;
	SHT20_SDA_H;
	for(i=0;i<8;i++ )
	{
		delay_us(2);
		SHT20_SCL_H;
		Rec <<= 1;
		if(I2C_SDA)
			Rec++;   
		delay_us(2);
		SHT20_SCL_L;		
	}     
	
	if (!ack)
		SHT20_Ack();//·¢ËÍnACK
	else
		SHT20_NAck(); //·¢ËÍACK   
	return Rec;
}

u8 SHT20_WriteByte(u8 byte)
{                        
	u8 i;   
	SHT20_SCL_L; 
	for(i=0;i<8;i++)
	{              
		if (byte & 0x80)  
		{
			SHT20_SDA_H;
		}
		else
		{
			SHT20_SDA_L;
		}
		byte <<= 1;    
		delay_us(2);   
		SHT20_SCL_H;
		delay_us(2); 
		SHT20_SCL_L; 
		delay_us(2);
	}  
	SHT20_SDA_H;
	delay_us(2);    
	SHT20_SCL_H;
	delay_us(2);
	if (I2C_SDA)
	{
		SHT20_SCL_L;
		return I2C_NACK;
	}
	else
	{
		SHT20_SCL_L;
		return I2C_ACK;
	}
	return 0;
}

void SHT20_Start(void)     
{
	SHT20_SDA_H;
	SHT20_SCL_H;
	delay_us(1);
	SHT20_SDA_L;
	delay_us(2);
	SHT20_SCL_L;
}

void SHT20_Stop(void)
{
	SHT20_SDA_L;
	SHT20_SCL_L;
	delay_us(4);
	SHT20_SCL_H;
	delay_us(4);
	SHT20_SDA_H;
	delay_us(4);
}

void SHT20_Ack(void)
{
	SHT20_SCL_L;
	SHT20_SDA_L;
	delay_us(2);
	SHT20_SCL_H;
	delay_us(2);
	SHT20_SCL_L;
}

void SHT20_NAck(void)
{
	SHT20_SCL_L;
	SHT20_SDA_H;
	delay_us(2);
	SHT20_SCL_H;
	delay_us(2);
	SHT20_SCL_L;
}


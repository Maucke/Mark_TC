#include  "ds3231.h"
#include  "uart.h"
#include  "delay.h"

/**************DS3231M�ӿ�����*****************/
sbit SCL = P1 ^ 6;
sbit SDA = P1 ^ 7;

bit	ack;
u8 FlagTimeGo = True;
u8 FlagAlarm = False;

u8 DS3231_Init_Buf[7] = { 0x55, 0x11, 0x10, 0x26, 0x01, 0x06, 0x18 };
u8 DS3231_Us_Buf[7];
	 

/********************************************************************************************************
** 	��������:			BCD2HEX(u8 val)
**	��������:			BCDתHEX
********************************************************************************************************/
u8	BCD2HEX(u8 val)
{
	return	((val >> 4) * 10) + (val & 0x0f);
}
/********************************************************************************************************
**	��������:			HEXתBCD
**	��ڲ���:			val:HAX��
********************************************************************************************************/
u8	HEX2BCD(u8 val)
{
	return	(((val % 100) / 10) << 4) | (val % 10);
}

void Start_I2C(void)
{
	SDA = High;                  //������ʼ�����������ź�
	SCL = High;
	delay_us(2);             //��ʼ��������ʱ�����4.7us,��ʱ     
	SDA = Low;                  //������ʼ�ź�
	delay_us(2);             // ��ʼ��������ʱ�����4��s		        
	SCL = Low;                  //ǯסI2C���ߣ�׼�����ͻ��������
}

void Stop_I2C(void)
{
	SDA = Low;                  //���ͽ��������������ź�
	SCL = High;                  //������������ʱ�����4us
	delay_us(2);
	SDA = High;                  //����I2C���߽����ź�
}

void DS3231_WriteByte(u8 c)
{
	u8 BitCnt;

	for (BitCnt = 0; BitCnt < 8; BitCnt++)         //Ҫ���͵����ݳ���Ϊ8λ
	{
		if ((c << BitCnt) & 0x80)
			SDA = 1;                          //�жϷ���λ
		else
			SDA = 0;
		SCL = High;                            //��ʱ����Ϊ�ߣ�֪ͨ��������ʼ��������λ
		delay_us(2);                       //��֤ʱ�Ӹߵ�ƽ���ڴ���4��s   
		SCL = Low;
	}
	SDA = High;                                  //8λ��������ͷ������ߣ�׼������Ӧ��λ
	SCL = High;
	delay_us(2);
	if (SDA == 1)
		ack = 0;
	else
		ack = 1;                              //�ж��Ƿ���յ�Ӧ���ź�
	SCL = Low;
	delay_us(2);
}

u8 DS3231_ReadByte(void)
{
	u8 retc;
	u8 BitCnt;

	retc = 0;
	SDA = High;                           //��������Ϊ���뷽ʽ
	for (BitCnt = 0; BitCnt < 8; BitCnt++)
	{
		SCL = Low;                      //��ʱ����Ϊ�ͣ�׼����������λ       
		delay_us(2);                 //ʱ�ӵ͵�ƽ���ڴ���4.7��s		       
		SCL = High;                      //��ʱ����Ϊ��ʹ��������������Ч
		delay_us(2);
		retc = retc << 1;
		if (SDA == 1)
			retc = retc + 1;            //������λ,���յ�����λ����retc��
		delay_us(2);
	}
	SCL = Low;
	return(retc);
}

void DS3231_Ack(bit a)
{
	SDA = a;
	SCL = High;
	delay_us(2);             //ʱ�ӵ͵�ƽ���ڴ���4��s    
	SCL = Low;                  //��ʱ���ߣ�ǯסI2C�����Ա��������
	delay_us(2);
}

u8 DS3231_WriteData(u8 addr, u8 write_data)
{
	Start_I2C();
	DS3231_WriteByte(DS3231_WriteAddress);
	if (ack == 0)
		return 0;

	DS3231_WriteByte(addr);
	if (ack == 0)
		return 0;

	DS3231_WriteByte(write_data);
	if (ack == 0)
		return 0;

	Stop_I2C();
	delay_us(4);
	return 1;
}

u8 DS3231_Current(void)
{
	u8 read_data;
	Start_I2C();
	DS3231_WriteByte(DS3231_ReadAddress);
	if (ack == 0)
		return(0);
	read_data = DS3231_ReadByte();
	DS3231_Ack(1);
	Stop_I2C();
	return read_data;
}

u8 DS3231_ReadData(u8 random_addr)
{
	Start_I2C();
	DS3231_WriteByte(DS3231_WriteAddress);
	if (ack == 0)
		return(0);
	DS3231_WriteByte(random_addr);
	if (ack == 0)
		return(0);
	return(DS3231_Current());
}

/********************************************************************************************************
** 	��������:			void	ModifyTime()
**	��������:			д�뵱ǰ���ں�ʱ��
**                      ����Ϊ��ʽ������
********************************************************************************************************/
void DS3231_SetTime(u8 *p)
{
	DS3231_WriteData(DS3231_YEAR, p[6]);   //�޸���
	DS3231_WriteData(DS3231_MONTH, p[5]);  //�޸���
	DS3231_WriteData(DS3231_DAY, p[4]);    //�޸���
	DS3231_WriteData(DS3231_WEEK, p[3]); //�޸���
	DS3231_WriteData(DS3231_HOUR, p[2]);   //�޸�ʱ
	DS3231_WriteData(DS3231_MINUTE, p[1]); //�޸ķ�
	DS3231_WriteData(DS3231_SECOND, p[0]); //�޸���
}

/********************************************************************************************************
** 	��������:			void	GetDateTime()
**	��������:			��ȡ��ǰ���ں�ʱ��
********************************************************************************************************/
void DS3231_GetTime(void)
{
	DS3231_Us_Buf[6] = DS3231_ReadData(DS3231_YEAR);	//��
	DS3231_Us_Buf[5] = DS3231_ReadData(DS3231_MONTH);	//��
	DS3231_Us_Buf[4] = DS3231_ReadData(DS3231_DAY);	//��
	DS3231_Us_Buf[3] = DS3231_ReadData(DS3231_WEEK);		//��
	DS3231_Us_Buf[2] = DS3231_ReadData(DS3231_HOUR) & 0x3f;	//ʱ 	
	DS3231_Us_Buf[1] = DS3231_ReadData(DS3231_MINUTE);//��
	DS3231_Us_Buf[0] = DS3231_ReadData(DS3231_SECOND);	//��
}

void DS3231_SetTimePrepar(u8 Type)
{
	if(Type == 0)
	{
		DS3231_Init_Buf[6] = DS3231_ReadData(DS3231_YEAR);	//��
		DS3231_Init_Buf[5] = DS3231_ReadData(DS3231_MONTH);	//��
		DS3231_Init_Buf[4] = DS3231_ReadData(DS3231_DAY);	//��
		DS3231_Init_Buf[3] = DS3231_ReadData(DS3231_WEEK);		//��
		DS3231_Init_Buf[2] = DS3231_ReadData(DS3231_HOUR) & 0x3f;	//ʱ 	
		DS3231_Init_Buf[1] = DS3231_ReadData(DS3231_MINUTE);//��
		DS3231_Init_Buf[0] = DS3231_ReadData(DS3231_SECOND);	//��
	}
	else if(Type == 1)
	{
		DS3231_Init_Buf[6] = DS3231_ReadData(DS3231_YEAR);	//��
		DS3231_Init_Buf[5] = DS3231_ReadData(DS3231_MONTH);	//��
		DS3231_Init_Buf[4] = DS3231_ReadData(DS3231_DAY);	//��
		DS3231_Init_Buf[3] = DS3231_ReadData(DS3231_WEEK);		//��
	}
	else if(Type == 2)
	{
		DS3231_Init_Buf[2] = DS3231_ReadData(DS3231_HOUR) & 0x3f;	//ʱ 	
		DS3231_Init_Buf[1] = DS3231_ReadData(DS3231_MINUTE);//��
		DS3231_Init_Buf[0] = DS3231_ReadData(DS3231_SECOND);	//��
	}
}

void DS3231_SetUart(void)
{
	if(Device_Msg.uartday&&Device_Msg.uartmonth&&Device_Msg.uartweek<=7)
	{
		DS3231_Init_Buf[0] = HEX2BCD(Device_Msg.uartsecond);//��
		DS3231_Init_Buf[1] = HEX2BCD(Device_Msg.uartminute);//��
		DS3231_Init_Buf[2] = HEX2BCD(Device_Msg.uarthour);//ʱ
		if (Device_Msg.uartweek == 0)
			DS3231_Init_Buf[3] = 7;//����
		else
			DS3231_Init_Buf[3] = HEX2BCD(Device_Msg.uartweek & 0x7);//����
		DS3231_Init_Buf[4] = HEX2BCD(Device_Msg.uartday);//��
		DS3231_Init_Buf[5] = HEX2BCD(Device_Msg.uartmonth);//��
		DS3231_Init_Buf[6] = HEX2BCD(Device_Msg.uartyear - 2000);//��
		DS3231_SetTime(DS3231_Init_Buf);
	}
}

void DS3231_Int() interrupt 2
{
//	RestCount ++;
//	FlagTimeGo = True;
	FlagAlarm = True;
}

u8 DS3231_Control;
void DS3231_Init(void)
{
//	IT1 = 0;                    //����INT1���ж�����Ϊ�����غ��½���,�����غ��½��ؾ��ɻ���
  IT1 = 1;                    //����INT1���ж�����Ϊ���½���,�½��ػ���

	EX1 = 1;                    //ʹ��INT1�ж�
	EA = 1;
	DS3231_Control = DS3231_ReadData(DS3231_CONTROL);
	
	DS3231_AlarmReset();
//	DS3231_SetTime(DS3231_Init_Buf);
}


void DS3231_AlarmReset(void)
{	    
	DS3231_WriteData(DS3231_STATUS,0x00);
	delay_ms(10);
	FlagAlarm = False;
}


void DS3231_AlarmSet(u8 Type,SW,Hour,Minute)
{	
	switch(Type)
	{
		case 1:
		DS3231_WriteData(DS3231_ALARM1SECOND,0x00);
		DS3231_WriteData(DS3231_ALARM1MINUTE,Minute);
		DS3231_WriteData(DS3231_ALARM1HOUR,Hour);
		DS3231_WriteData(DS3231_ALARM1WEEK,0x80);
		if(SW)
			DS3231_Control|=0x05;
		else
			DS3231_Control&=~0x01;
		DS3231_WriteData(DS3231_CONTROL,DS3231_Control);
		break;
		case 2:
		DS3231_WriteData(DS3231_ALARM2MINUTE,Minute);
		DS3231_WriteData(DS3231_ALARM2HOUR,Hour);
		if(SW)
			DS3231_Control|=0x06;
		else
			DS3231_Control&=~0x02;
		DS3231_WriteData(DS3231_CONTROL,DS3231_Control);break;
	}

	
	DS3231_AlarmReset();
}		

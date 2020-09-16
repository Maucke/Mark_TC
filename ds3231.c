#include  "ds3231.h"
#include  "uart.h"
#include  "delay.h"

/**************DS3231M接口设置*****************/
sbit SCL = P1 ^ 6;
sbit SDA = P1 ^ 7;

bit	ack;
u8 FlagTimeGo = True;
u8 FlagAlarm = False;

u8 DS3231_Init_Buf[7] = { 0x55, 0x11, 0x10, 0x26, 0x01, 0x06, 0x18 };
u8 DS3231_Us_Buf[7];
	 

/********************************************************************************************************
** 	函数名称:			BCD2HEX(u8 val)
**	功能描述:			BCD转HEX
********************************************************************************************************/
u8	BCD2HEX(u8 val)
{
	return	((val >> 4) * 10) + (val & 0x0f);
}
/********************************************************************************************************
**	功能描述:			HEX转BCD
**	入口参数:			val:HAX码
********************************************************************************************************/
u8	HEX2BCD(u8 val)
{
	return	(((val % 100) / 10) << 4) | (val % 10);
}

void Start_I2C(void)
{
	SDA = High;                  //发送起始条件的数据信号
	SCL = High;
	delay_us(2);             //起始条件建立时间大于4.7us,延时     
	SDA = Low;                  //发送起始信号
	delay_us(2);             // 起始条件锁定时间大于4μs		        
	SCL = Low;                  //钳住I2C总线，准备发送或接收数据
}

void Stop_I2C(void)
{
	SDA = Low;                  //发送结束条件的数据信号
	SCL = High;                  //结束条件建立时间大于4us
	delay_us(2);
	SDA = High;                  //发送I2C总线结束信号
}

void DS3231_WriteByte(u8 c)
{
	u8 BitCnt;

	for (BitCnt = 0; BitCnt < 8; BitCnt++)         //要传送的数据长度为8位
	{
		if ((c << BitCnt) & 0x80)
			SDA = 1;                          //判断发送位
		else
			SDA = 0;
		SCL = High;                            //置时钟线为高，通知被控器开始接收数据位
		delay_us(2);                       //保证时钟高电平周期大于4μs   
		SCL = Low;
	}
	SDA = High;                                  //8位发送完后释放数据线，准备接收应答位
	SCL = High;
	delay_us(2);
	if (SDA == 1)
		ack = 0;
	else
		ack = 1;                              //判断是否接收到应答信号
	SCL = Low;
	delay_us(2);
}

u8 DS3231_ReadByte(void)
{
	u8 retc;
	u8 BitCnt;

	retc = 0;
	SDA = High;                           //置数据线为输入方式
	for (BitCnt = 0; BitCnt < 8; BitCnt++)
	{
		SCL = Low;                      //置时钟线为低，准备接收数据位       
		delay_us(2);                 //时钟低电平周期大于4.7μs		       
		SCL = High;                      //置时钟线为高使数据线上数据有效
		delay_us(2);
		retc = retc << 1;
		if (SDA == 1)
			retc = retc + 1;            //读数据位,接收的数据位放入retc中
		delay_us(2);
	}
	SCL = Low;
	return(retc);
}

void DS3231_Ack(bit a)
{
	SDA = a;
	SCL = High;
	delay_us(2);             //时钟低电平周期大于4μs    
	SCL = Low;                  //清时钟线，钳住I2C总线以便继续接收
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
** 	函数名称:			void	ModifyTime()
**	功能描述:			写入当前日期和时间
**                      星期为公式计算结果
********************************************************************************************************/
void DS3231_SetTime(u8 *p)
{
	DS3231_WriteData(DS3231_YEAR, p[6]);   //修改年
	DS3231_WriteData(DS3231_MONTH, p[5]);  //修改月
	DS3231_WriteData(DS3231_DAY, p[4]);    //修改日
	DS3231_WriteData(DS3231_WEEK, p[3]); //修改周
	DS3231_WriteData(DS3231_HOUR, p[2]);   //修改时
	DS3231_WriteData(DS3231_MINUTE, p[1]); //修改分
	DS3231_WriteData(DS3231_SECOND, p[0]); //修改秒
}

/********************************************************************************************************
** 	函数名称:			void	GetDateTime()
**	功能描述:			获取当前日期和时间
********************************************************************************************************/
void DS3231_GetTime(void)
{
	DS3231_Us_Buf[6] = DS3231_ReadData(DS3231_YEAR);	//年
	DS3231_Us_Buf[5] = DS3231_ReadData(DS3231_MONTH);	//月
	DS3231_Us_Buf[4] = DS3231_ReadData(DS3231_DAY);	//日
	DS3231_Us_Buf[3] = DS3231_ReadData(DS3231_WEEK);		//周
	DS3231_Us_Buf[2] = DS3231_ReadData(DS3231_HOUR) & 0x3f;	//时 	
	DS3231_Us_Buf[1] = DS3231_ReadData(DS3231_MINUTE);//分
	DS3231_Us_Buf[0] = DS3231_ReadData(DS3231_SECOND);	//秒
}

void DS3231_SetTimePrepar(u8 Type)
{
	if(Type == 0)
	{
		DS3231_Init_Buf[6] = DS3231_ReadData(DS3231_YEAR);	//年
		DS3231_Init_Buf[5] = DS3231_ReadData(DS3231_MONTH);	//月
		DS3231_Init_Buf[4] = DS3231_ReadData(DS3231_DAY);	//日
		DS3231_Init_Buf[3] = DS3231_ReadData(DS3231_WEEK);		//周
		DS3231_Init_Buf[2] = DS3231_ReadData(DS3231_HOUR) & 0x3f;	//时 	
		DS3231_Init_Buf[1] = DS3231_ReadData(DS3231_MINUTE);//分
		DS3231_Init_Buf[0] = DS3231_ReadData(DS3231_SECOND);	//秒
	}
	else if(Type == 1)
	{
		DS3231_Init_Buf[6] = DS3231_ReadData(DS3231_YEAR);	//年
		DS3231_Init_Buf[5] = DS3231_ReadData(DS3231_MONTH);	//月
		DS3231_Init_Buf[4] = DS3231_ReadData(DS3231_DAY);	//日
		DS3231_Init_Buf[3] = DS3231_ReadData(DS3231_WEEK);		//周
	}
	else if(Type == 2)
	{
		DS3231_Init_Buf[2] = DS3231_ReadData(DS3231_HOUR) & 0x3f;	//时 	
		DS3231_Init_Buf[1] = DS3231_ReadData(DS3231_MINUTE);//分
		DS3231_Init_Buf[0] = DS3231_ReadData(DS3231_SECOND);	//秒
	}
}

void DS3231_SetUart(void)
{
	if(Device_Msg.uartday&&Device_Msg.uartmonth&&Device_Msg.uartweek<=7)
	{
		DS3231_Init_Buf[0] = HEX2BCD(Device_Msg.uartsecond);//秒
		DS3231_Init_Buf[1] = HEX2BCD(Device_Msg.uartminute);//分
		DS3231_Init_Buf[2] = HEX2BCD(Device_Msg.uarthour);//时
		if (Device_Msg.uartweek == 0)
			DS3231_Init_Buf[3] = 7;//星期
		else
			DS3231_Init_Buf[3] = HEX2BCD(Device_Msg.uartweek & 0x7);//星期
		DS3231_Init_Buf[4] = HEX2BCD(Device_Msg.uartday);//日
		DS3231_Init_Buf[5] = HEX2BCD(Device_Msg.uartmonth);//月
		DS3231_Init_Buf[6] = HEX2BCD(Device_Msg.uartyear - 2000);//年
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
//	IT1 = 0;                    //设置INT1的中断类型为上升沿和下降沿,上升沿和下降沿均可唤醒
  IT1 = 1;                    //设置INT1的中断类型为仅下降沿,下降沿唤醒

	EX1 = 1;                    //使能INT1中断
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

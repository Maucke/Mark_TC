#include "irf.h"
#include "delay.h"

/*************	用户系统配置	**************/

#define MAIN_Fosc		22118400L	//定义主时钟, 红外接收会自动适应。5~36MHZ

#define D_TIMER0		125			//选择定时器时间, us, 红外接收要求在60us~250us之间

#define	User_code		0xFD02		//定义红外接收用户码

#define freq_base			(MAIN_Fosc / 1200)
#define Timer0_Reload		(65536 - (D_TIMER0 * freq_base / 10000))
u16 IRF_REV;				//高8位为系统码，低八位为数据码

u8 FlagIRF = False;

sbit	IRIN = P3^2;		//定义红外接收输入端口

bit		IRIN_temp;		//Last sample
bit		B_IR_Sync;			//已收到同步标志
u8	IR_SampleCnt;		//采样计数
u8	IR_BitCnt;			//编码位数
u8	IR_UserH;			//用户码(地址)高字节
u8	IR_UserL;			//用户码(地址)低字节
u8	IR_data;			//数据原码
u8	IR_DataShit;		//数据反码

bit		B_IrUserErr;		//User code error flag

//*******************************************************************
//*********************** IR Remote Module **************************

//*********************** IR Remote Module **************************
//this programme is used for Receive IR Remote (HT6121).

//data format: Synchro,AddressH,AddressL,data,/data, (total 32 bit).

//send a frame(85ms), pause 23ms, send synchro of another frame, pause 94ms

//data rate: 108ms/Frame


//Synchro:low=9ms,high=4.5/2.25ms,low=0.5626ms
//Bit0:high=0.5626ms,low=0.5626ms
//Bit1:high=1.6879ms,low=0.5626ms
//frame space = 23 ms or 96 ms

/******************** 红外采样时间宏定义, 用户不要随意修改	*******************/

#if ((D_TIMER0 <= 250) && (D_TIMER0 >= 60))
	#define	D_IR_sample			D_TIMER0		//定义采样时间，在60us~250us之间
#endif

#define D_IR_SYNC_MAX		(15000/D_IR_sample)	//SYNC max time
#define D_IR_SYNC_MIN		(9700 /D_IR_sample)	//SYNC min time
#define D_IR_SYNC_DIVIDE	(12375/D_IR_sample)	//decide data 0 or 1
#define D_IR_DATA_MAX		(3000 /D_IR_sample)	//data max time
#define D_IR_DATA_MIN		(600  /D_IR_sample)	//data min time
#define D_IR_DATA_DIVIDE	(1687 /D_IR_sample)	//decide data 0 or 1
#define D_IR_BIT_NUMBER		32					//bit number


/********************** Timer0中断函数************************/
void IRF_GetValue (void) interrupt 12
{
	u8	SampleTime;

	IR_SampleCnt++;							//Sample + 1

	F0 = IRIN_temp;						//Save Last sample status
	IRIN_temp = IRIN;					//Read current status
	if(F0 && !IRIN_temp)					//Last sample is high，and current sample is low, so is fall edge
	{
		SampleTime = IR_SampleCnt;			//get the sample time
		IR_SampleCnt = 0;					//Clear the sample counter

			 if(SampleTime > D_IR_SYNC_MAX)		B_IR_Sync = 0;	//large the Maxim SYNC time, then error
		else if(SampleTime >= D_IR_SYNC_MIN)					//SYNC
		{
			if(SampleTime >= D_IR_SYNC_DIVIDE)
			{
				B_IR_Sync = 1;					//has received SYNC
				IR_BitCnt = D_IR_BIT_NUMBER;	//Load bit number
			}
		}
		else if(B_IR_Sync)						//has received SYNC
		{
			if(SampleTime > D_IR_DATA_MAX)		B_IR_Sync=0;	//data samlpe time to large
			else
			{
				IR_DataShit >>= 1;					//data shift right 1 bit
				if(SampleTime >= D_IR_DATA_DIVIDE)	IR_DataShit |= 0x80;	//devide data 0 or 1
				if(--IR_BitCnt == 0)				//bit number is over?
				{
					B_IR_Sync = 0;					//Clear SYNC
					if(~IR_DataShit == IR_data)		//判断数据正反码
					{
						if((IR_UserH == (User_code / 256)) &&
							IR_UserL == (User_code % 256))
								B_IrUserErr = 0;	//User code is righe
						else	B_IrUserErr = 1;	//user code is wrong
							
						IRF_REV      = IR_data;
						FlagIRF   = 1;			//数据有效
					}
				}
				else if((IR_BitCnt & 7)== 0)		//one byte receive
				{
					IR_UserL = IR_UserH;			//Save the User code high byte
					IR_UserH = IR_data;				//Save the User code low byte
					IR_data  = IR_DataShit;			//Save the IR data byte
				}
			}
		}
	}
}
void IEF_Int() interrupt 0     //INT0中断入口
{
}

/**************** Timer初始化函数 ******************************/
void IRF_Init(void)
{
	AUXR |= 0x04;		//定时器时钟1T模式
	T2L = 0x33;		//设置定时初值
	T2H = 0xF5;		//设置定时初值
	AUXR |= 0x10;		//定时器2开始计时
	IE2 |= 0x04;
	
	IRIN = 1;
	IT0 = 1;                    //设置INT0的中断类型 (1:仅下降沿 0:上升沿和下降沿)
	EX0 = 1;                    //使能INT0中断
	EA  = 1;
}



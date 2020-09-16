#include "irf.h"
#include "delay.h"

/*************	�û�ϵͳ����	**************/

#define MAIN_Fosc		22118400L	//������ʱ��, ������ջ��Զ���Ӧ��5~36MHZ

#define D_TIMER0		125			//ѡ��ʱ��ʱ��, us, �������Ҫ����60us~250us֮��

#define	User_code		0xFD02		//�����������û���

#define freq_base			(MAIN_Fosc / 1200)
#define Timer0_Reload		(65536 - (D_TIMER0 * freq_base / 10000))
u16 IRF_REV;				//��8λΪϵͳ�룬�Ͱ�λΪ������

u8 FlagIRF = False;

sbit	IRIN = P3^2;		//��������������˿�

bit		IRIN_temp;		//Last sample
bit		B_IR_Sync;			//���յ�ͬ����־
u8	IR_SampleCnt;		//��������
u8	IR_BitCnt;			//����λ��
u8	IR_UserH;			//�û���(��ַ)���ֽ�
u8	IR_UserL;			//�û���(��ַ)���ֽ�
u8	IR_data;			//����ԭ��
u8	IR_DataShit;		//���ݷ���

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

/******************** �������ʱ��궨��, �û���Ҫ�����޸�	*******************/

#if ((D_TIMER0 <= 250) && (D_TIMER0 >= 60))
	#define	D_IR_sample			D_TIMER0		//�������ʱ�䣬��60us~250us֮��
#endif

#define D_IR_SYNC_MAX		(15000/D_IR_sample)	//SYNC max time
#define D_IR_SYNC_MIN		(9700 /D_IR_sample)	//SYNC min time
#define D_IR_SYNC_DIVIDE	(12375/D_IR_sample)	//decide data 0 or 1
#define D_IR_DATA_MAX		(3000 /D_IR_sample)	//data max time
#define D_IR_DATA_MIN		(600  /D_IR_sample)	//data min time
#define D_IR_DATA_DIVIDE	(1687 /D_IR_sample)	//decide data 0 or 1
#define D_IR_BIT_NUMBER		32					//bit number


/********************** Timer0�жϺ���************************/
void IRF_GetValue (void) interrupt 12
{
	u8	SampleTime;

	IR_SampleCnt++;							//Sample + 1

	F0 = IRIN_temp;						//Save Last sample status
	IRIN_temp = IRIN;					//Read current status
	if(F0 && !IRIN_temp)					//Last sample is high��and current sample is low, so is fall edge
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
					if(~IR_DataShit == IR_data)		//�ж�����������
					{
						if((IR_UserH == (User_code / 256)) &&
							IR_UserL == (User_code % 256))
								B_IrUserErr = 0;	//User code is righe
						else	B_IrUserErr = 1;	//user code is wrong
							
						IRF_REV      = IR_data;
						FlagIRF   = 1;			//������Ч
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
void IEF_Int() interrupt 0     //INT0�ж����
{
}

/**************** Timer��ʼ������ ******************************/
void IRF_Init(void)
{
	AUXR |= 0x04;		//��ʱ��ʱ��1Tģʽ
	T2L = 0x33;		//���ö�ʱ��ֵ
	T2H = 0xF5;		//���ö�ʱ��ֵ
	AUXR |= 0x10;		//��ʱ��2��ʼ��ʱ
	IE2 |= 0x04;
	
	IRIN = 1;
	IT0 = 1;                    //����INT0���ж����� (1:���½��� 0:�����غ��½���)
	EX0 = 1;                    //ʹ��INT0�ж�
	EA  = 1;
}



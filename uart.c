#include "Uart.h"

u8 Uart_Overflow_Flag = False;
u8 Uart_Recv_Step = 0;

u8 Uart_Down_Ready = False;
u8 Uart_Down_Step = 0;

u8 Uart_Recv_Count = 0;  
u8 Uart_Recv_Length = 0;
u8 Uart_Recv_Data = 0;
u8 Uart_Recv_BUF[Uart_Max_Length];

DEVICE_MSG Device_Msg;
#define DATA_PACKAGE_MIN_LEN    5
#define DATA_PACKAGE_MAX_LEN    512
#define DATA_PACKAGE_FFT_LEN    200

#define CMD_HEAD1 0xFF
#define CMD_HEAD2 0x55

#define MAKEWORD(low, high)    (((u8)(low)) | (((u8)(high)) << 8))

void Send_Data(u8 dat)
{ 
	SBUF = dat; 			//�����յ����ݷ��ͻ�ȥ
	while(!TI);							//��鷢���жϱ�־λ
		TI = 0; 							//����жϱ�־λΪ0��������㣩
}
void uart_tx_string(u8 *str)
{
	while(*str!='\0')
	{
		Send_Data(*str++);
	}
}


void Uart_Init()
{
	SCON = 0x50;		//8λ����,�ɱ䲨����
	AUXR |= 0x40;		//��ʱ��1ʱ��ΪFosc,��1T
	AUXR &= 0xFE;		//����1ѡ��ʱ��1Ϊ�����ʷ�����
	TMOD &= 0x0F;		//�趨��ʱ��1Ϊ16λ�Զ���װ��ʽ
	TL1 = 0xD0;		//�趨��ʱ��ֵ
	TH1 = 0xFF;		//�趨��ʱ��ֵ
	ET1 = 0;		//��ֹ��ʱ��1�ж�
	TR1 = 1;		//������ʱ��1
  ES = 1;            
	
  EA = 1;  
}

u8 ReponseID[]={0xFF,0x55,'O','K',0x06,'M','a','r','k','T','C'};
void Analysismsg(u8* Buf)
{
    int i;
//    if (Buf[0] == CMD_HEAD1 && Buf[1] == CMD_HEAD2)
    {
        {
            switch (MAKEWORD(Buf[3], Buf[2]))
            {
							case Uart_Year:
								Device_Msg.uartyear = MAKEWORD(Buf[6],Buf[5]);
								break;
							case Uart_Month:
								Device_Msg.uartmonth = MAKEWORD(Buf[6],Buf[5]);
								break;
							case Uart_Day:
								Device_Msg.uartday = MAKEWORD(Buf[6],Buf[5]);
								break;
							case Uart_Week:
								Device_Msg.uartweek = MAKEWORD(Buf[6],Buf[5]);
								break;
							case Uart_Hour:
								Device_Msg.uarthour = MAKEWORD(Buf[6],Buf[5]);
								break;
							case Uart_Minute:
								Device_Msg.uartminute = MAKEWORD(Buf[6],Buf[5]);
								break;
							case Uart_Second:
								Device_Msg.uartsecond = MAKEWORD(Buf[6],Buf[5]);Uart_Overflow_Flag=1;
								break;
							case 0x3F3F:
								ReponseID[2] = (Buf[5] & 0x5A) | (Buf[6] & 0xA5);
								ReponseID[3] = (Buf[5] ^ 0x57) | (Buf[6] & 0x57);
					
								uart_tx_string(ReponseID);
							break;
            }
        }
    }
}

void Uart_Int(void) interrupt 4
{  
	u8 Uart_Recv_Data = 0;
	if(RI)  
	{  
		RI = False; 
		Uart_Recv_Data = SBUF;
//									Send_Data(Uart_Recv_Data);
		if(Uart_Recv_Data == 0x7F)
		{
			if(Uart_Down_Step++ > 4)
			{
				Uart_Down_Step = 0;
				Uart_Down_Ready = True;
			}
		}
		else
			Uart_Down_Step = 0;
		
   	if(!Uart_Overflow_Flag)
		{
        if (!Uart_Overflow_Flag)
        {
            switch (Uart_Recv_Step)
            {
                case 0: if (Uart_Recv_Data == CMD_HEAD1) Uart_Recv_Step++; break;
                case 1: if (Uart_Recv_Data == CMD_HEAD2) Uart_Recv_Step++; else Uart_Recv_Step = 0; break;
                case 2: Uart_Recv_BUF[2] = Uart_Recv_Data; Uart_Recv_Step++; break;
                case 3: Uart_Recv_BUF[3] = Uart_Recv_Data; Uart_Recv_Step++; break;
                case 4: Uart_Recv_BUF[4] = Uart_Recv_Data; Uart_Recv_Step++; break;
                case 5: Uart_Recv_BUF[Uart_Recv_Count + DATA_PACKAGE_MIN_LEN] = Uart_Recv_Data; Uart_Recv_Count++; if (Uart_Recv_Count >= Uart_Recv_BUF[4]) { Uart_Recv_Step = 0; Uart_Recv_Count = 0; Analysismsg(Uart_Recv_BUF); } break;
            }
        }
		}
	}     
}

	
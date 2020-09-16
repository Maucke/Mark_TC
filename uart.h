#ifndef __UART_H_
#define __UART_H_

#include "sys.h"
#define Uart_Year 	 			 	0x201
#define Uart_Month 					0x202
#define Uart_Day 	  				0x203
#define Uart_Week 					0x204
#define Uart_Hour 	  			0x205
#define Uart_Minute 				0x206
#define Uart_Second 	  		0x207
typedef struct
{
	u16 cputemp;
	u16 cpuclock;
	u16 cpuload;
	u16 cpufan;
	u16 cpupower;

	u16 gputemp;
	u16 gpuclock;
	u16 gpuload;
	u16 gpufan;

	u16 maintemp;
	u16 mainfan;
	u16 mainvoltage;

	u16 ramload;
	u16 ramusrdata;

	u16 hddload;
	u16 hddtemp;

	u16 uartyear;
	u16 uartmonth;
	u16 uartday;
	u16 uartweek;
	u16 uarthour;
	u16 uartminute;
	u16 uartsecond;

	char cpuname[40];
	char gpuname[40];
	char mainname[40];
	u16 leftvol;
	u16 rightvol;
}DEVICE_MSG;

extern DEVICE_MSG Device_Msg;
#define Uart_Safe_Mode 1
#define Uart_Test_Mode 1

#define Uart_Max_Length 16
#define Uart_Data_Length 20

#define End_Frame_Flag 0x1234

extern u8 Uart_Overflow_Flag;
extern u8 Uart_Down_Start;
extern u8 Uart_Down_Second;
extern u8 Uart_Down_Ready;
extern u8 Uart_Recv_BUF[Uart_Max_Length];

void Send_Data(u8 UART_data);
void Uart_Init(void);
void Uart_Test();

#endif
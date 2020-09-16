#include "sys.h"
#include "manualdown12c5a.h"
#include "ds3231.h"
#include "adc.h"
#include "pt6311.h"
#include "uart.h"
#include "sht20.h"
#include "eeprom.h"
#include "irf.h" 
#include "rgb.h" 

u16 DisCount = 0;
u16 DisNumCount = 0;
u16 RestCount = 0;
u16 AlmCount = 0;
u16 DisMode = 0;
u16 ECOCount = 0;
u16 RGBNagr = 0;
u8 RGBArray[3]={0,0,0};

u8 MenuIndex = 0;
u8 MenuValue = 0;
u16 TempFix = 0;

bit FlagECO = False;
bit FlagFlash = True;
bit FlagRefrush = True;
bit FlagAutoUSB = True;
bit FlagCTPress = False;
bit FlagSWPress = False;
//sbit Blink = P3^4;

sbit Key_CT = P3^6;
sbit Key_SW = P3^7;
sbit Beep = P4^0;

#define RESTDEFINE 120
#define VERSION "C2_22"

#define AddrDIM 0x0000
#define AddrTmMt 0x0001
#define AddrMiMt 0x0002
#define AddrBlnk 0x0003
#define AddrSWRGB  0x0004

#define AddrSWALM 0x0005
#define AddrSWECO 0x0006

#define AddrALMH 0x0007
#define AddrALMM 0x0008
#define AddrECOH 0x0009
#define AddrECOM 0x000A

#define AddrCODE 0x0020
//#define EcoMode 0x0001
//#define AddrSpsM 0x0007
//#define AddrEpsH 0x0008
//#define AddrEpsM 0x0009
u8 EEPROM_Buf[E2Length] = {
	0x09,0x02,0x02,0x01,0x03,0x00,0x01,0x07,0x30,0x08,0x00,
//DIM ,TmMT,MiMt,Blnk,SRGB,SALM,SECO,SpsH,SpsM,EpsH,EpsM,
};

u8 EEPROM_Def[E2Length] = {
	0x09,0x02,0x02,0x01,0x03,0x00,0x01,0x07,0x30,0x08,0x00,
//DIM ,TmMT,MiMt,Blnk,SRGB,SALM,SECO,SpsH,SpsM,EpsH,EpsM,
};

void DecInt() interrupt 16          //INT3中断入口
{

}

void CTInt() interrupt 10
{
	u8 i=0;
	for(i=0;i<20;i++);
	if(Key_CT == 0)
	{
		FlagCTPress = True;
	}
}

void SWInt() interrupt 11
{
	u8 i=0;
	for(i=0;i<20;i++);
	if(Key_SW == 0)
	{
		FlagSWPress = True;
	}
}


void RGB_Init(u16 Time,u8 Color)
{
	u8 i;
	RGBNagr = Time;
	for(i=0;i<3;i++)
		RGBArray[i] = Color;
}

void EEPROM_Init(void)
{
	u8 i;
	if(IapReadByte(AddrCODE)!=0x5A)
	{
		IapEraseSector(0);
		EEPROM_Def[AddrCODE] = 0x5A;
		for (i = 0; i<E2Length; i++)
			IapProgramByte(i, EEPROM_Def[i]);
		PT6311_MiStr("HELLO");DisCount=50;
	}
	for (i = 0; i<E2Length; i++)
		EEPROM_Buf[i] = IapReadByte(i);
}

void EEPROM_Process(void)
{
	u8 i;
	IapEraseSector(0);
	for (i = 0; i<E2Length; i++)
		IapProgramByte(i, EEPROM_Buf[i]);
}

void GPIO_Config(void)
{
  P1ASF = 0x01;        
	P1M1 = 0xC1;		
	P1M0 = 0xFE;   
	
//	P1M1 = 0x00;		
//	P1M0 = 0xFF;
}

u8 DimValue = 0;

u8 Modify_Brightness(u16 L)  //根据传感器数值设置亮度
{
	if (L >= 1010)
	{
		DimValue = 1;RGB_Init(20,10);
	}
	else if (L >= 960&&L <= 1005)
	{
		DimValue = 2;MaxRGBLium = 12;
	}
	else if (L >= 890&&L <= 950)
	{
		DimValue = 3;MaxRGBLium = 24;
	}
	else if (L >= 550&&L <= 880)
	{
		DimValue = 4;MaxRGBLium = 48;
	}
	else if (L >= 400&&L <= 540)
	{
		DimValue = 5;MaxRGBLium = 56;
	}
	else if (L >= 250&&L <= 390)
	{
		DimValue = 6;MaxRGBLium = 64;
	}
	else if (L >= 100&&L <= 240)
	{
		DimValue = 7;MaxRGBLium = 240;
	}
	else if (L <= 95)
	{
		DimValue = 8;MaxRGBLium = 255;
	}
	return DimValue;
}

u16 DimADC;

void Dim_Control(u8 Level)
{
	DimADC = GetADCResult(0);
	if(Level < 9)
	{
		PT6311_Dim(Level);
		switch(Level)
		{
			case 1:RGB_Init(20,10);break;
			case 2:MaxRGBLium = 12;break;
			case 3:MaxRGBLium = 24;break;
			case 4:MaxRGBLium = 48;break;
			case 5:MaxRGBLium = 56;break;
			case 6:MaxRGBLium = 64;break;
			case 7:MaxRGBLium = 240;break;
			case 8:MaxRGBLium = 255;break;
		}
		PT6311_ReFush(VCH,0);
	}
	else
	{
		PT6311_Dim(Modify_Brightness(DimADC));
		PT6311_ReFush(VCH,1);
	}
}

void Run_Init(void)		//2毫秒@22.1184MHz
{
	AUXR |= 0x80;		//定时器时钟1T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0x33;		//设置定时初值 他
	TH0 = 0x53;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
	PT0 = 1;
	ET0 = 1;
	EA = 1;
}

void Btn_Init(void)
{
  INT_CLKO |= 0x20;           //(EX3 = 1)使能INT3下降沿中断
	INT_CLKO |= 0x10; 
	EA = 1;
}

void Run_Int() interrupt 1
{
	static u16 RunCount = 0;
	static u16 TmgCount = 0;
	static u16 TepCount = 0;
	static u16 RGBCount = 0;
	if(RunCount++>12)
	{
		RunCount = 0;
		FlagRefrush = True;
	}
	if(TepCount++>28)
	{	
		TepCount = 0;
		if(!RGBNagr)
		{
			switch(EEPROM_Buf[AddrSWRGB])
			{
				case 0:RGB_Init(20,10);break;
				case 1:RGB1_Breath(RGBCount++);break;
				case 2:RGB1_Wheel(RGBCount++);break;
				case 3:RGB1_WheelS(RGBCount++);break;
			}
		}
		else
		{
			RGB1_Refrash(RGBArray[0],RGBArray[1],RGBArray[2]);
			RGBNagr--;
		}
	}
	if(TmgCount++>248)
	{
		TmgCount = 0;
		RestCount ++;
		if(EEPROM_Buf[AddrBlnk])
		{
			FlagFlash = !FlagFlash;
		}
		FlagTimeGo = True;
		if(EEPROM_Buf[AddrSWECO])
			if(DimADC>1018)
			{
				if(ECOCount++>3598)
				{
					FlagECO = True;
					ECOCount = 0;
				}
			}
			else
			{
				ECOCount = 0;
			}
	}	
}

void Option_SW(u8 Title)
{
	if(DisCount==0)
	{
		switch (Title)
		{
		case 0:PT6311_PutYear(DS3231_Us_Buf[6]); break;
		case 1:PT6311_PutMonDay(DS3231_Us_Buf[5], DS3231_Us_Buf[4]); break;
		case 2:PT6311_PutWeek(DS3231_Us_Buf[3]); break;
		case 3:PT6311_PutTemp(SHT20_Us_Buf[1] - TempFix/1000); break;
		case 4:PT6311_PutHumi(SHT20_Us_Buf[0] + TempFix/500); break;
		case 5:PT6311_PutDim(EEPROM_Buf[AddrDIM]); break;
		case 6:PT6311_PutMiMt(EEPROM_Buf[AddrMiMt]); break;
		case 7:PT6311_PutTmMt(EEPROM_Buf[AddrTmMt]); break;
		}
	}
	else
		DisCount--;
}

void Time_SW(void)
{
	if(DisNumCount==0)
		PT6311_PtTime(DS3231_Us_Buf[2], DS3231_Us_Buf[1], DS3231_Us_Buf[0]);
	else
		DisNumCount--;
}


u8 Contrast(u8 Input,u16 Object)
{
	if(Input>(Object&0x3F))
		return Input-- ;
	else if(Input<(Object&0x3F))
		return Input++ ;
	else
		return Input ;
}

u16 SpeedControl = 0;
void MainDisplay()
{
	Time_SW();
	PT6311_TimeMt(EEPROM_Buf[AddrTmMt],FlagFlash);
	if(SpeedControl%2==0)
	{
		PT6311_DoEvent();
	}
	PT6311_CYCLEMt();
	
	if(SpeedControl%2==0)
	{
		PT6311_Tilte(Step_Mt[7],0);
		Option_SW(Step_Mt[7]);
		PT6311_MiMt(EEPROM_Buf[AddrMiMt]);
	}
	SpeedControl++;
}

void Mode_Show(u8 *ch,u16 Time)
{
	u8 i;
	PT6311_MiStr(ch);
	PT6311_DobReFush(VNETWORK,0x8000);
	for(i=0;i<6;i++)
	{
		while(!FlagRefrush);
		PT6311_MiMt(EEPROM_Buf[AddrMiMt]);
		FlagRefrush = False;
		while(!FlagRefrush);
		PT6311_DobReFush(VBOXDN,rand());
		FlagRefrush = False;
	}
	PT6311_DobReFush(VBOXDN,0xFFFF);
	delay_ms(Time);
}

void Time_Show(u8 Hou, Min, Sec,u16 Time)
{
	u8 i;
	PT6311_PtTime(Hou, Min, Sec);
	for(i=0;i<8;i++)
	{
		while(!FlagRefrush);
		PT6311_TimeMt(0,FlagFlash);
		FlagRefrush = False;
	}
	delay_ms(Time);
}

void DeviceMsg(void)
{
//	TI = 1;
//	
//	printf("IRF:%X\r\n",IRF_REV);
//	printf("ADC:%d\r\n",GetADCResult(0));
//	printf("RestCount:%d\r\n",RestCount);
//	printf("TempFix:%d\r\n",TempFix);
//	printf("ECOCount:%d\r\n",ECOCount);
//	printf("WEEK:%d\r\n",DS3231_Us_Buf[5]);
//	printf("SEC:%d\r\n",DS3231_Us_Buf[0]);
//	printf("MIN:%d\r\n",DS3231_Us_Buf[1]);
//	printf("HOU:%d\r\n",DS3231_Us_Buf[2]);
//	while(!TI);							//检查发送中断标志位
//		TI = 0; 							//令发送中断标志位为0（软件清零）
}

void Detec_Down()
{
	if(Uart_Down_Ready)
	{
		PT6311_Init();
		Mode_Show("LOAD",1);
		PT6311_ReFush(VUSB, 0xFF);
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		ISP_CONTR=0x60;
		
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
//		Uart_Down_Ready = False;
	}
}

void Mode_Sleep()
{
	RGB_Init(20,0);
	PT6311_Init();
	Mode_Show("SLEEP",500);
	PT6311_State(0);
	delay_ms(500);
	EA = 0;
	RGB1_Write(0,0,0);
	RGB1_Write(0,0,0);
	EA = 1;
	PCON |= 0x02;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	RGB_Init(100,MaxRGBLium);
//	DS3231_AlarmReset();
	DS3231_GetTime();	
//	if(DS3231_Us_Buf[2] == EEPROM_Buf[AddrSpsH] && DS3231_Us_Buf[1] == EEPROM_Buf[AddrSpsM])
//	{
//		FlagECO = False;
//		PCON |= 0x02;
//		_nop_();
//		_nop_();
//		_nop_();
//		_nop_();
//	}

	PT6311_Init();
	Mode_Show("WAKE ",1);
	SHT20_HostReadValue();
	DisCount=50;
	SpeedControl = 0;
	FlagCTPress = False;//防止按键唤醒出现bug
	FlagSWPress = False;//防止按键唤醒出现bug
	FlagRefrush = True;
	FlagTimeGo = True;
}

void Set_BCDCK(u8 Type,Func,Min,Max)
{
	u8 Temporary ;
	Temporary = BCD2HEX(DS3231_Init_Buf[Type]);
	if(Func)
	{
		if(Temporary++ > (Max-1)) Temporary = Min;
	}
	else
	{
		if(Temporary-- < (Min+1)) Temporary = Max;
	}
	DS3231_Init_Buf[Type] = HEX2BCD(Temporary);
}

void Set_BCDE2(u8 Type,Func,Min,Max)
{
	u8 Temporary ;
	Temporary = BCD2HEX(EEPROM_Buf[Type]);
	if(Func)
	{
		if(Temporary++ > (Max-1)) Temporary = Min;
	}
	else
	{
		if(Temporary-- < (Min+1)) Temporary = Max;
	}
	EEPROM_Buf[Type] = HEX2BCD(Temporary);
}

void Set_Config(u8 Type,Func)
{
	switch(Type)
	{
		case 0x00:Set_BCDCK(6,Func,18,99);break;
		case 0x01:Set_BCDCK(4,Func,1,12);break;
		case 0x02:Set_BCDCK(3,Func,1,31);break;
		case 0x03:Set_BCDCK(5,Func,1,7);break;
		
		case 0x10:Set_BCDCK(2,Func,0,23);break;
		case 0x11:Set_BCDCK(1,Func,0,59);break;
		case 0x12:Set_BCDCK(0,Func,0,59);break;
		
		case 0x20:Set_BCDE2(AddrALMH,Func,0,23);break;
		case 0x21:Set_BCDE2(AddrALMM,Func,0,59);break;
		case 0x22:Set_BCDE2(AddrECOH,Func,0,23);break;
		case 0x23:Set_BCDE2(AddrECOM,Func,0,59);break;
		
		case 0x30:Set_BCDE2(AddrBlnk,Func,0,1);break;
		case 0x31:Set_BCDE2(AddrSWRGB,Func,0,4);break;
		case 0x32:Set_BCDE2(AddrSWECO,Func,0,1);break;
		case 0x33:Set_BCDE2(AddrSWALM,Func,0,1);break;
		
	}
}

void IRF_Control(u8 KeyValue)
{
	switch(KeyValue)
	{
		case IRF_RT:Step_Mt[8]=Step_Mt[7];if(Step_Mt[7]++>3) Step_Mt[7] = 0;Step_Mt[4]=0;Step_Mt[1]=0;DisCount=0;break;
		case IRF_LF:Step_Mt[8]=Step_Mt[7];if(Step_Mt[7]--<1) Step_Mt[7] = 4;Step_Mt[4]=0;Step_Mt[1]=0;DisCount=0;break;
		case IRF_HOME:Step_Mt[8]=Step_Mt[7];Step_Mt[7] = 0;Step_Mt[4]=0;Step_Mt[1]=0;DisCount=0;break;
		
		case IRF_MSG:PT6311_MiStr(VERSION);DisCount=50;break;
		case IRF_MUTE:EEPROM_Buf[AddrBlnk]=(~EEPROM_Buf[AddrBlnk])&1;PT6311_Blink(EEPROM_Buf[AddrBlnk]);if(!EEPROM_Buf[AddrBlnk]) FlagFlash=1;DisCount=50;EEPROM_Process(); break;
		
		case IRF_UP:if(EEPROM_Buf[AddrDIM]++>8) EEPROM_Buf[AddrDIM]=1;Step_Mt[7]=5;Step_Mt[4]=0;Step_Mt[1]=0;Dim_Control(EEPROM_Buf[AddrDIM]);DisCount=0;EEPROM_Process();break;
		case IRF_DN:if(EEPROM_Buf[AddrDIM]--<2) EEPROM_Buf[AddrDIM]=9;Step_Mt[7]=5;Step_Mt[4]=0;Step_Mt[1]=0;Dim_Control(EEPROM_Buf[AddrDIM]);DisCount=0;EEPROM_Process();break;
		
		case IRF_VUP:if(EEPROM_Buf[AddrMiMt]++>1) EEPROM_Buf[AddrMiMt]=1;Step_Mt[7]=6;Step_Mt[4]=0;Step_Mt[1]=0;DisCount=0;EEPROM_Process();break;
		case IRF_VDN:if(EEPROM_Buf[AddrMiMt]--<2) EEPROM_Buf[AddrMiMt]=2;Step_Mt[7]=6;Step_Mt[4]=0;Step_Mt[1]=0;DisCount=0;EEPROM_Process();break;
			
		case IRF_CUP:if(EEPROM_Buf[AddrTmMt]++>5) EEPROM_Buf[AddrTmMt]=1;Step_Mt[7]=7;Step_Mt[4]=0;Step_Mt[1]=0;DisCount=0;EEPROM_Process();break;
		case IRF_CDN:if(EEPROM_Buf[AddrTmMt]--<2) EEPROM_Buf[AddrTmMt]=6;Step_Mt[7]=7;Step_Mt[4]=0;Step_Mt[1]=0;DisCount=0;EEPROM_Process();break;
		
		case IRF_OFF:Mode_Sleep();break;
		case IRF_SET:DS3231_SetTimePrepar(0);DisMode = (~DisMode)&1;Step_Mt[7] = 0;Step_Mt[11] = 0;MenuIndex = 0; break;
		case IRF_GOTO:ISP_CONTR=0x20;break;
		
		case IRF_RED:Step_Mt[8]=Step_Mt[7];Step_Mt[7] = 1;Step_Mt[4]=0;Step_Mt[1]=0;DisCount=0;break;
		case IRF_YELLOW:Step_Mt[8]=Step_Mt[7];Step_Mt[7] = 2;Step_Mt[4]=0;Step_Mt[1]=0;DisCount=0;break;
		case IRF_GREEN:Step_Mt[8]=Step_Mt[7];Step_Mt[7] = 3;Step_Mt[4]=0;Step_Mt[1]=0;DisCount=0;break;
		case IRF_BLUE:Step_Mt[8]=Step_Mt[7];Step_Mt[7] = 4;Step_Mt[4]=0;Step_Mt[1]=0;DisCount=0;break;
	}
}

void Out_Set(void)
{
	DisMode = 0;
	Step_Mt[7]=0;
	Step_Mt[8]=Step_Mt[7];
	Step_Mt[4]=0;
	Step_Mt[1]=0;
	DisCount = 0;
	EEPROM_Init();
}

void Set_AlmConfig(void)
{
	DS3231_AlarmSet(1,EEPROM_Buf[AddrSWALM],EEPROM_Buf[AddrALMH],EEPROM_Buf[AddrALMM]);
	DS3231_AlarmSet(2,EEPROM_Buf[AddrSWECO],EEPROM_Buf[AddrECOH],EEPROM_Buf[AddrECOM]);
}

void Set_Control(u8 KeyValue)
{
	switch(KeyValue)
	{
		case IRF_RT:
			if(MenuIndex == 0) {if(Step_Mt[7]++>3) Step_Mt[7] = 0;Step_Mt[11] = 0;} 
			else if(Step_Mt[7]==0|Step_Mt[7]==2|Step_Mt[7]==3) {if(Step_Mt[11]++>2) Step_Mt[11] = 0;}
			else if(Step_Mt[7]==1) {if(Step_Mt[11]++>1) Step_Mt[11] = 0;}break;
			
		case IRF_LF:
			if(MenuIndex == 0) {if(Step_Mt[7]--<1) Step_Mt[7] = 4;Step_Mt[11] = 0;} 
			else if(Step_Mt[7]==0|Step_Mt[7]==2|Step_Mt[7]==3) {if(Step_Mt[11]--<1) Step_Mt[11] = 3;}
			else if(Step_Mt[7]==1) {if(Step_Mt[11]--<1) Step_Mt[11] = 2;}break;
		
		case IRF_UP:Set_Config(MenuValue,1); break;
		case IRF_DN:Set_Config(MenuValue,0); break;
		
		case IRF_OK:if(MenuIndex == 0&&Step_Mt[7]!=4) MenuIndex = 1;
			else if(Step_Mt[7]==0) {DS3231_SetTimePrepar(2);DS3231_SetTime(DS3231_Init_Buf); PT6311_MiStr("SUCES");Out_Set();DisCount=50;}
			else if(Step_Mt[7]==1) {DS3231_SetTimePrepar(1);DS3231_SetTime(DS3231_Init_Buf); PT6311_MiStr("SUCES");Out_Set();DisCount=50;}
			else if(Step_Mt[7]==2) {EEPROM_Process();Set_AlmConfig(); PT6311_MiStr("SUCES");Out_Set();DisCount=50;}
			else if(Step_Mt[7]==3) {EEPROM_Process(); PT6311_MiStr("SUCES");Out_Set();DisCount=50;}
			else if(Step_Mt[7]==4) {IapEraseSector(0);EEPROM_Init();Set_AlmConfig(); PT6311_MiStr("SUCES");Out_Set();DisCount=50;}break;
			
		case IRF_BACK:if(MenuIndex == 1) MenuIndex = 0;else Out_Set(); break;
//		DS3231_SetTime(DS3231_Init_Buf);PT6311_MiStr("SUCES");DisCount=50;DisMode = 0;
		case IRF_HOME:Out_Set();break;
		case IRF_MSG:PT6311_MiStr(VERSION);DisCount=50;break;
		
		case IRF_OFF:Out_Set();Mode_Sleep();break;
		case IRF_SET:DisMode = (~DisMode)&1;Out_Set(); break;
		case IRF_GOTO:ISP_CONTR=0x20;break;
		
		case IRF_RED:Step_Mt[8]=Step_Mt[7];Step_Mt[7] = 1;Step_Mt[4]=0;Step_Mt[1]=0;DisCount=0;break;
		case IRF_YELLOW:Step_Mt[8]=Step_Mt[7];Step_Mt[7] = 2;Step_Mt[4]=0;Step_Mt[1]=0;DisCount=0;break;
		case IRF_GREEN:Step_Mt[8]=Step_Mt[7];Step_Mt[7] = 3;Step_Mt[4]=0;Step_Mt[1]=0;DisCount=0;break;
		case IRF_BLUE:Step_Mt[8]=Step_Mt[7];Step_Mt[7] = 4;Step_Mt[4]=0;Step_Mt[1]=0;DisCount=0;break;
	}
}


void IRF_Process(void)
{
	if(FlagIRF)
	{
		if(FlagAlarm)
		{
			FlagAlarm = False;
			DS3231_AlarmReset();
			AlmCount = 0;
			Beep = OFF;
			FlagIRF = False;
		}		
		else 
		{
			IRF_Control(IRF_REV);
			FlagIRF = False;
		}
	}
}


void IRF_Set(void)
{
	if(FlagIRF)
	{
		if(FlagAlarm)
		{
			FlagAlarm = False;
			DS3231_AlarmReset();
			AlmCount = 0;
			Beep = OFF;
			FlagIRF = False;
		}		
		else 
		{
			Set_Control(IRF_REV);
			FlagIRF = False;
		}
	}
}

void CT_Happen()
{
	static u16 SleepRun = 0;

	if(FlagCTPress)
	{
		if(!Key_CT)
		{
			if(SleepRun++ >150)
			{
				SleepRun = 0;
				FlagCTPress = False;
				Mode_Sleep();
			}
		}
		else if(FlagAlarm)
		{
			FlagAlarm = False;
			DS3231_AlarmReset();
			AlmCount = 0;
			Beep = OFF;
			FlagCTPress = False;
		}		
		else
		{
			Step_Mt[8]=Step_Mt[7];if(Step_Mt[7]++>3) Step_Mt[7] = 0;Step_Mt[4]=0;Step_Mt[1]=0;DisCount=0;
			FlagCTPress = False;
		}
	}
	else
		SleepRun = 0;
}

bit FlagBeep = False;

void Sec_Process(void)
{
	static u8 Sec_SpendControl = 0;
	if(FlagTimeGo)
	{
		if(Sec_SpendControl%2==0)
			PT6311_NETMt(0);
		SHT20_ReadValue();
		DS3231_GetTime();
		Dim_Control(EEPROM_Buf[AddrDIM]);
		DeviceMsg();
		Sec_SpendControl++;
		if(TempFix<10000)
			TempFix ++;
		FlagTimeGo = False;
//		beep = !beep;
	}
}

void SW_Happen()
{
	static u16 SleepRun = 0;

	if(FlagSWPress)
	{
		if(!Key_SW)
		{
			if(SleepRun++ >150)
			{
				SleepRun = 0;
				FlagSWPress = False;
				Mode_Sleep();
			}
		}
		else if(FlagAlarm)
		{
			FlagAlarm = False;
			DS3231_AlarmReset();
			AlmCount = 0;
			Beep = OFF;
			FlagSWPress = False;
		}		
		else
		{
			if(EEPROM_Buf[AddrDIM]++>8) EEPROM_Buf[AddrDIM]=1;Step_Mt[7]=5;Step_Mt[4]=0;Step_Mt[1]=0;Dim_Control(EEPROM_Buf[AddrDIM]);DisCount=0;EEPROM_Process();
			FlagSWPress = False;
		}
	}
	else
		SleepRun = 0;
	
}

u8 Voice_Buf[16]={0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,};

void Put_Process(void)
{
	if(FlagRefrush)
	{			
		MainDisplay();
		Detec_Down();
		CT_Happen();
		SW_Happen();
		if(FlagAlarm)
		{
			if((DS3231_Us_Buf[2]==EEPROM_Buf[AddrECOH])&&(DS3231_Us_Buf[1]==EEPROM_Buf[AddrECOM])&&AlmCount == 0)
			{
				DS3231_AlarmReset();
				FlagAlarm = False;
				Beep = OFF;
			}
			else
			{
				Beep = Voice_Buf[AlmCount%16];
				if(AlmCount++>2000)
				{
					AlmCount = 0;
					FlagAlarm = False;
					Beep = OFF;
				}
			}
		}
		FlagRefrush = False;
	}		
}

void PutSetValue(u8 Type)
{
	switch(Type)
	{
		case 0x00:PT6311_PtTime(0xA2,DS3231_Init_Buf[6]>>4,((DS3231_Init_Buf[6]<<4)&0xF0)|0x0A);break;
		case 0x01:PT6311_PtTime(0xAA,DS3231_Init_Buf[4],0xAA);break;
		case 0x02:PT6311_PtTime(0xAA,DS3231_Init_Buf[3],0xAA);break;
		case 0x03:PT6311_PtTime(0xAA,DS3231_Init_Buf[5],0xAA);break;
		
		case 0x10:PT6311_PtTime(0xAA,DS3231_Init_Buf[2],0xAA);break;
		case 0x11:PT6311_PtTime(0xAA,DS3231_Init_Buf[1],0xAA);break;
		case 0x12:PT6311_PtTime(0xAA,DS3231_Init_Buf[0],0xAA);break;
		
		case 0x20:PT6311_PtTime(0xAA,EEPROM_Buf[AddrALMH],0xAA);break;
		case 0x21:PT6311_PtTime(0xAA,EEPROM_Buf[AddrALMM],0xAA);break;
		case 0x22:PT6311_PtTime(0xAA,EEPROM_Buf[AddrECOH],0xAA);break;
		case 0x23:PT6311_PtTime(0xAA,EEPROM_Buf[AddrECOM],0xAA);break;
		
		case 0x30:PT6311_Bool(EEPROM_Buf[AddrBlnk]);break;
		case 0x31:PT6311_Bool(EEPROM_Buf[AddrSWRGB]);break;
		case 0x32:PT6311_Bool(EEPROM_Buf[AddrSWECO]);break;
		case 0x33:PT6311_Bool(EEPROM_Buf[AddrSWALM]);break;
	}
}

void Option_Set(u8 Title)
{
	switch(Title)
	{
		case 0:PT6311_MiStr("DATE ");break;
		case 1:PT6311_MiStr("TIME ");break;
		case 2:PT6311_MiStr("ALARM");break;
		case 3:PT6311_MiStr("CONFG");break;
		case 4:PT6311_MiStr("RESTO");break;
	}
}

void Inside_Set(u8 Title)
{
	switch(Title)
	{
		case 0x00:PT6311_MiStr("YEAR ");break;
		case 0x01:PT6311_MiStr("MONTH");break;
		case 0x02:PT6311_MiStr("DAY  ");break;
		case 0x03:PT6311_MiStr("WEEK ");break;
		
		case 0x10:PT6311_MiStr("HOUR ");break;
		case 0x11:PT6311_MiStr("MINTE");break;
		case 0x12:PT6311_MiStr("SECND");break;
		
		case 0x20:PT6311_MiStr("ALM_H");break;
		case 0x21:PT6311_MiStr("ALM_M");break;
		case 0x22:PT6311_MiStr("ECO_H");break;
		case 0x23:PT6311_MiStr("ECO_M");break;
		
		case 0x30:PT6311_MiStr("BLINK");break;
		case 0x31:PT6311_MiStr("RGBSW");break;
		case 0x32:PT6311_MiStr("ECOSW");break;
		case 0x33:PT6311_MiStr("ALMSW");break;
	}
}

void MainSet(void)
{
	MenuValue = ((Step_Mt[7]<<4)|(Step_Mt[11]&0xF))&0xFF;
	if(SpeedControl%2==0)
	{
		PT6311_DoSet();
	}
	if(MenuIndex == 0)
	{
		PT6311_PtTime(0xAB,0xBB,0xBA);
	}
	else
		PutSetValue(MenuValue);
	PT6311_TimeMt(EEPROM_Buf[AddrTmMt],0);
	
	if(SpeedControl%2==0)
	{
		if(MenuIndex == 0)
		{
			PT6311_Tilte(Step_Mt[7],0);
			Option_Set(Step_Mt[7]);
		}
		else
			Inside_Set(MenuValue);
		PT6311_MiMt(EEPROM_Buf[AddrMiMt]);
	}
	SpeedControl++;
}

void Put_Set(void)
{
	if(FlagRefrush)
	{			
		MainSet();
		Detec_Down();
		CT_Happen();
		SW_Happen();
		FlagRefrush = False;
	}		
}

void Rest_Process(void)
{
	if(RestCount>=RESTDEFINE*60)
	{
		RGB_Init(200,0);
		PT6311_Init();
		Mode_Show("REST ",500);
		PT6311_State(0);
//		Blink = OFF;
		delay_ms(10000);
		RGB_Init(40,MaxRGBLium);
		PT6311_Init();
		PT6311_Check(50);
		Mode_Show("WAKE  ",500);
		SHT20_HostReadValue();
		DisCount=50;
		SpeedControl = 0;
		FlagRefrush = True;
		FlagTimeGo = True;
		RestCount = 0;
		DS3231_AlarmReset();
	}
}

void Sleep_Process(void)
{
	if(FlagECO)
	{
//		DS3231_AlarmReset();
//		DS3231_GetTime();	
//		if(DS3231_Us_Buf[2] == EEPROM_Buf[AddrSpsH]&&DS3231_Us_Buf[1] == EEPROM_Buf[AddrSpsM])
//		{
		Mode_Sleep();
//		}
		TempFix = 0;
		FlagECO = False;
	}
}

void Uart_Process(void)		//Uart
{
	u8 i;
	u8 CoCount = 0;
	if(Uart_Overflow_Flag&&FlagAutoUSB)
	{
		PT6311_ReFush(VROUND, 0xFF);
		PT6311_ReFush(VUSB, 0xFF);
		Time_Show(0xAA,0xAA,0xAA,1);
		Mode_Show("USB  ", 1000);
		Mode_Show("READY", 1);
		for(i=0;i<30;i++)
		{
			if (Uart_Overflow_Flag)
			{
				Uart_Overflow_Flag = False;
				CoCount++;
			}
			delay_ms(50);
		}
		if(CoCount>1)
		{
			DS3231_SetUart();		//校准时间
			Mode_Show("CPLET", 1000);
			FlagAutoUSB = False;
		}
		else
		{
			Mode_Show("FAULT", 1000);
		}
		PT6311_ReFush(VUSB, 0x00);
		DisCount = 50;
		SpeedControl = 0;
		FlagRefrush = True;
		FlagTimeGo = True;
		Uart_Overflow_Flag = False;
	}
	else if(Uart_Overflow_Flag)
		Uart_Overflow_Flag = False;
}

void Self_Check(void)
{
	Step_Mt[4] = 8;//中间转盘比较值
	Step_Mt[7] = 255;//标题位置
	Step_Mt[9] = 8;//中间转轮当前位置
	Step_Mt[10] = 1;
	EEPROM_Buf[AddrBlnk] = 0;
	while(Step_Mt[10])
	{
		while(!FlagRefrush);
		PT6311_InitEvent();
		PT6311_CYCLEMt();
		Time_SW();
		PT6311_TimeMt(EEPROM_Buf[AddrTmMt],FlagFlash);
		
		if(SpeedControl%2==0)
		{
			PT6311_InitNet();
			PT6311_Tilte(Step_Mt[7],0);
			Option_SW(Step_Mt[7]);
			PT6311_MiMt(EEPROM_Buf[AddrMiMt]);
//			Blink = !Blink;
		}
		SpeedControl++;
		FlagRefrush = False;
	}
	Step_Mt[2] = 6;//左边转盘起始位置
	Step_Mt[3] = 0;
}



int main(void)
{
	IspControl();															//下载控制
	RGB1_send_L();
	GPIO_Config();
	Uart_Init();
	DS3231_Init();
	InitSHT20();
	IRF_Init();
	Run_Init();
	Btn_Init();
	SHT20_HostReadValue();
	PT6311_Init();
	PT6311_MiStr("MARK ");DisCount=50;
	DS3231_GetTime();	
	EEPROM_Init();
	Dim_Control(EEPROM_Buf[AddrDIM]);
	RGB_Init(40,MaxRGBLium);
	Self_Check();
	EEPROM_Init();
	Set_AlmConfig();
	INT_CLKO |= 0x40; //开启接收中断
	
	while(1)
	{
		switch(DisMode)
		{
			case 0:
			Put_Process();
			Sec_Process();
			Rest_Process();
			Uart_Process();
			IRF_Process();
			Sleep_Process();break;
			case 1:
			Put_Set();
			Sec_Process();
			IRF_Set();break;
		}
		delay_ms(2);
	}
}
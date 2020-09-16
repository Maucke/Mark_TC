#ifndef __PT6311_FUTABA_H_
#define __PT6311_FUTABA_H_

#include "sys.h"
#include "uart.h"
#include "delay.h"

extern u8 code NumTab[];
extern u8 code HRDRTab[];
extern u16 code MiTab[];
extern u8 code CYCLETab[];
extern u8 code AutoTab[];
extern u8 Step_Mt[];
extern u8 DimCount;

void PT6311_State(u8 State);
void PT6311_Wrt(u8 dat);
void PT6311_Func(void);
void PT6311_Mode(void);
void PT6311_Start(void);
void PT6311_Dim(u8 Light);
void PT6311_Init(void);
void PT6311_ReFush(u8 Addr, u8 dat);
void PT6311_DobReFush(u8 Addr, u16 dat);
void PT6311_NumDis(u8 *Sp);
void PT6311_NumRAM(u8 Flash);
void PT6311_MiDis(u16 *Sp);
void PT6311_MiRAM(void);
void PT6311_MiStr(u8 *ch);
void PT6311_NETMt(u8 Style);
void PT6311_PtTime(u8 Hou, Min, Sec);
void PT6311_TimeMt(u8 Style,u8 Flash);
void PT6311_MiMt(u8 Style);
void PT6311_DoEvent(void);
void PT6311_CYCLEMt(void);
void PT6311_Tilte(u8 Type, u8 Error);
void PT6311_PutYear(u8 Year);
void PT6311_PutMonDay(u8 Month, u8 Day);
void PT6311_PutWeek(u8 Week);
void PT6311_PutTemp(u8 Temp);
void PT6311_PutHumi(u8 Humi);
void PT6311_PutDim(u8 Dim);
void PT6311_PutMiMt(u8 MiMt);
void PT6311_PutTmMt(u8 TmMt);
void PT6311_DoSet(void);
void PT6311_Blink(u8 Val);
void PT6311_Check(u16 Speed);
void PT6311_InitEvent(void);
void PT6311_InitNet(void);
void PT6311_Bool(u8 Bl);
//#define SMILE			 0x9652
#define SMILE			 0x5A5A

//ADDR
#define VAUTO			 20
#define VEPG			 23
#define VTRANS		 17
#define VMAIL 		 32
#define VRATE 		 11

#define VHRDR 		 25		//HDD REC DVD REC ....  
#define VROUND 		 24
#define VMUSIC 		 31
#define VNETWORK 	 30
#define VUSB			 29

#define VTITLE 		 14
#define VCHP 			 8
#define VTRK			 5
#define VCH				 2

#define VMISEG0 	 12
#define VMISEG1 	 9
#define VMISEG2		 6
#define VMISEG3		 3
#define VMISEG4		 0

#define VNUMSEG0 	 15
#define VNUMSEG1 	 21
#define VNUMSEG2	 22
#define VNUMSEG3	 18
#define VNUMSEG4	 19
#define VNUMSEG5	 16

#define VBOXUP		 28
#define VBOXDN	 	 27
/* !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^-'abcdefghijklmnopqrstuvwxyz*/

#endif
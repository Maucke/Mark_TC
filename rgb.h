#ifndef __RGB_H
#define __RGB_H	

#include "sys.h"

extern u8 MaxRGBLium;
void RGB1_Write(unsigned char data1,data2,data3);
void RGB1_All(unsigned char data1,data2,data3);
void RGB1_Wheel(u16 WheelPos);
void RGB1_send_L(void);
void RGB1_send_H(void);
void RGB1_Refrash(u8 VR,u8 VG,u8 VB);
void RGB1_Breath(u16 WheelPos);
void RGB1_WheelS(u16 WheelPos) ;
#endif
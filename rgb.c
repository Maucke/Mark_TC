#include "rgb.h"
#include "sys.h"
sbit L_RGB = P0^3;
void RGB1_send_L(void)
{
	L_RGB=1;_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();
				L_RGB=0;_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();
}

void RGB1_send_H(void)
{
	L_RGB=1;_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();
		L_RGB = 0;			_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();

//			_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();
//					_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();
}

void RGB1_Write(unsigned char data1,data2,data3)
{  
	unsigned char i;
	
	for(i=0;i<8;i++) 
	{
		if(data2&0x80)
			RGB1_send_H();
		else 
			RGB1_send_L();                  
		data2<<=1;    
	} 

	for(i=0;i<8;i++) 
	{
		if(data1&0x80)
			RGB1_send_H();
		else 
			RGB1_send_L();                  
		data1<<=1;    
	}

	for(i=0;i<8;i++) 
	{
		if(data3&0x80)
			RGB1_send_H();
		else 
			RGB1_send_L();                
		data3<<=1;    
	}
}


u8 MaxRGBLium = 64;

void RGB1_Wheel(u16 WheelPos) 
{
	WheelPos = WheelPos%(MaxRGBLium*3);
	if(WheelPos < MaxRGBLium) 
	{
		RGB1_Refrash(0, MaxRGBLium - WheelPos, WheelPos);
	}
	else if(WheelPos < (MaxRGBLium*2)) 
	{          
		RGB1_Refrash(WheelPos-MaxRGBLium, 0, MaxRGBLium*2 - WheelPos);
	}
	else
	{
		RGB1_Refrash((MaxRGBLium*3) - WheelPos, WheelPos-(MaxRGBLium*2),0);
	}
}

void RGB1_WheelS(u16 WheelPos) 
{
	WheelPos = WheelPos%(MaxRGBLium*3);
	if(WheelPos < MaxRGBLium) 
	{
		RGB1_Refrash(MaxRGBLium/3, MaxRGBLium - WheelPos, WheelPos);
	}
	else if(WheelPos < (MaxRGBLium*2)) 
	{          
		RGB1_Refrash(WheelPos-MaxRGBLium, MaxRGBLium/3, MaxRGBLium*2 - WheelPos);
	}
	else
	{
		RGB1_Refrash((MaxRGBLium*3) - WheelPos, WheelPos-(MaxRGBLium*2),MaxRGBLium/3);
	}
}

void RGB1_Breath(u16 WheelPos)
{
	WheelPos = WheelPos%(MaxRGBLium*3);
	if(WheelPos < MaxRGBLium)
		RGB1_Refrash(WheelPos, WheelPos, WheelPos);
//	else
	else if(WheelPos < (MaxRGBLium*2)) 
		RGB1_Refrash(MaxRGBLium*2-WheelPos-1, MaxRGBLium*2-WheelPos-1, MaxRGBLium*2-WheelPos-1);
}

void RGB1_Refrash(u8 VR,u8 VG,u8 VB)
{
	static u8 R=0,G=0,B=0;
	if(VR>R)
		R++;
	else if(VR<R)
		R--;
	if(VG>G)
		G++;
	else if(VG<G)
		G--;
	if(VB>B)
		B++;
	else if(VB<B)
		B--;
	RGB1_Write(R,G,B);
}
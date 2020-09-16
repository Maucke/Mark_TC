#ifndef __MANUALDOWN_H__
#define __MANUALDOWN_H__

sfr ISP_CONTR=0xc7;

void IspControl()
{
	if((PCON&0x10)==0) //如果POF位=0
	{
		PCON=PCON|0x10;  //将POF位置1
		ISP_CONTR=0x60;  //软复位,从ISP监控区启动
	}
	else
	{
		PCON=PCON&0xef;  //将POF位清零
	}
}

#endif
#ifndef __MANUALDOWN_H__
#define __MANUALDOWN_H__

sfr ISP_CONTR=0xc7;

void IspControl()
{
	if((PCON&0x10)==0) //���POFλ=0
	{
		PCON=PCON|0x10;  //��POFλ��1
		ISP_CONTR=0x60;  //��λ,��ISP���������
	}
	else
	{
		PCON=PCON&0xef;  //��POFλ����
	}
}

#endif
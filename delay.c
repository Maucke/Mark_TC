#include "delay.h"

void delay_ms(unsigned int Ms)
{
	unsigned char i, j;
	do
	{
		i = 15;
		j = 90;
		do
		{
			while (--j);
		} while (--i);
	}while(--Ms);
}

//—” ±, Œ¢√Î
void delay_us(u16 i)
{
	int j,k;
	for(j=0;j<i;j++)
		for(k=0;k<3;k++);
}

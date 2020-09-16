#include "adc.h"
#include "delay.h"

/*Define ADC operation const for ADC_CONTR*/
#define ADC_POWER   0x80            //ADC power control bit
#define ADC_FLAG    0x10            //ADC complete flag
#define ADC_START   0x08            //ADC start control bit
#define ADC_SPEEDLL 0x00            //420 clocks
#define ADC_SPEEDL  0x20            //280 clocks
#define ADC_SPEEDH  0x40            //140 clocks
#define ADC_SPEEDHH 0x60            //70 clocks
#define ID_ADDR_ROM 0xeff7      //60K³ÌÐò¿Õ¼äµÄMCU

u16 GetADCResult(u8 ch)
{
	u32 SingValue=0;
	u8 i;
	for(i=0;i<100;i++)
	{
		ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ch | ADC_START;
		_nop_();                        //Must wait before inquiry
		_nop_();
		_nop_();
		_nop_();
		while (!(ADC_CONTR & ADC_FLAG));//Wait complete flag
		ADC_CONTR &= ~ADC_FLAG;         //Close ADC
		SingValue+= ADC_RES << 2 | ADC_RESL & 0x3;             //Return ADC result
	}
	return SingValue/100;
}

u16 GetADCDefine(u8 ch)
{
	u16 SingValue=0;
	ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ch | ADC_START;
	_nop_();                        //Must wait before inquiry
	_nop_();
	_nop_();
	_nop_();
	while (!(ADC_CONTR & ADC_FLAG));//Wait complete flag
	ADC_CONTR &= ~ADC_FLAG;         //Close ADC
	SingValue+= ADC_RES << 2 | ADC_RESL & 0x3;             //Return ADC result
	return SingValue;
}

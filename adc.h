#ifndef __ADC_H_
#define __ADC_H_

#include "STC15.h"
#include "sys.h"
#include "intrins.h"

void InitADC();
u16 GetADCResult(u8 ch);
u16 GetADCDefine(u8 ch);

#endif
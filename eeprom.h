#ifndef __EEPROM_H__
#define __EEPROM_H__

#include "sys.h"
#include "delay.h"

void IapIdle();
u8 IapReadByte(u16 addr);
void IapProgramByte(u16 addr, u8 dat);
void IapEraseSector(u16 addr);

#endif
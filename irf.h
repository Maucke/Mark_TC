#ifndef __IRF_H_
#define __IRF_H_

#include "sys.h"

extern u16 IRF_REV;
extern u8 FlagIRF;

void IRF_Init(void);

#define IRF_LF 0x19
#define IRF_RT 0x18
#define IRF_UP 0x16
#define IRF_DN 0x17
#define IRF_OK 0x15

#define IRF_OFF 0x0D
#define IRF_SET 0x98
#define IRF_MUTE 0x0E

#define IRF_SHOW 0x90
#define IRF_MOVE 0x91
#define IRF_GAME 0xA8
#define IRF_APP 0x93

#define IRF_HOME 0x94
#define IRF_MSG 0x95

#define IRF_BACK 0x48
#define IRF_MENU 0x14

#define IRF_VUP 0x44
#define IRF_VDN 0x43

#define IRF_CUP 0x4A
#define IRF_CDN 0x4B

#define IRF_JLF 0x58
#define IRF_JRT 0x59
#define IRF_GOTO 0x12

#define IRF_RED 0x62
#define IRF_YELLOW 0x64
#define IRF_GREEN 0x63
#define IRF_BLUE 0x65


#endif
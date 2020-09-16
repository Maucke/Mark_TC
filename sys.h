#ifndef __SYS_H_
#define __SYS_H_

#include "stc15.h"
#include "intrins.h"
#include <stdio.h>
#include <stdlib.h> 

#define u8 unsigned char
#define u16 unsigned int
#define u32 long unsigned int


sbit KEY_Menu = P3 ^ 3;


//sbit KEY_Left = P0 ^ 1;
//sbit KEY_Right = P0 ^ 3;
//sbit KEY_Up = P0 ^ 0;
//sbit KEY_Down = P0 ^ 2;

#define True 1
#define False 0

#define High 1
#define Low 0      

#define OFF 1
#define ON 0     

#endif
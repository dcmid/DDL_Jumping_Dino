/*
===============================================================================
 Name        : dino_jump.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

// TODO: insert other include files here

#define FIO0DIR (*(volatile unsigned int*)0x2009C000) //pin direction reg
#define PINMODE0 (*(volatile unsigned int*)0x4002C040) //pin resistor mode reg
#define FIO0PIN0 (*(volatile unsigned int*)0x2009C014) //pin value reg

//#define EXTINT (*(volatile unsigned int*)0x400FC140) //external interrupt flag reg
//#define EXTMODE (*(voaltile unsigned int*)0x400FC148) //external interrupt mode reg
//#define EXTPOLAR (*(volatile unsigned int*)0x400FC14C) //external interrupt polarity reg


#define SCK ((FIO0PIN0 >> 0) & 1)
#define SDA ((FIO0PIN0 >> 1) & 1)

//void busout_8bit(unsigned int output){
//	for(int i=0; i<32; i++){
//		if((x>>i)&1)
//			FIO0PIN |= (1<<offset[i]);
//		else
//			FIO0PIN &= ~(1<<offset[i]);
//	}
//}

int main(void) {

    // TODO: insert code here

    // Force the counter to be placed into memory
    volatile static int i = 0 ;
    // Enter an infinite loop, just incrementing a counter
    while(1) {
        i++ ;
    }
    return 0 ;
}

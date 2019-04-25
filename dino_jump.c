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
#include <stdio.h>

// TODO: insert other include files here

#define FIO0DIR (*(volatile unsigned int*)0x2009C000) //pin direction reg
#define PINMODE0 (*(volatile unsigned int*)0x4002C040) //pin resistor mode reg
#define FIO0PIN0 (*(volatile unsigned int*)0x2009C014) //pin value reg

#define IO0IntEnF (*(volatile unsigned int*)0x40028094)
#define IO0IntClr (*(volatile unsigned int*)0x4002808C)
#define ISER0 (*(volatile unsigned int*)0xE000E100)

#define SCK ((FIO0PIN0 >> 0) & 1) //SCK (ps2 clock) is at P0.0 (pin 9)
#define SDA ((FIO0PIN0 >> 1) & 1) //SDA (ps2 data) is at P0.1 (pin 10)

#define SPACEBAR 0b01001010001

//void busout_8bit(unsigned int output){
//	for(int i=0; i<32; i++){
//		if((x>>i)&1)
//			FIO0PIN |= (1<<offset[i]);
//		else
//			FIO0PIN &= ~(1<<offset[i]);
//	}
//}

int num_clocks = 0;
int space_pressed = 0;
unsigned int ps2_data = 0;

void shift_in(int bit_in){
	ps2_data = ps2_data<<1; //shift ps2_data
	ps2_data |= (bit_in<<0); //insert bit_in
	ps2_data &= 0b11111111111; //preserve only bottom 11 bits
}

void EINT3_IRQHandler(void){//executes on falling edge of SCK (ps2 clock)
	shift_in(SDA);//shift SDA into ps2_data
	num_clocks++;//increment ps2 clock count
	if(num_clocks == 11){//when 11 bits have been read, compare to spacebar encoding
		space_pressed = (ps2_data == SPACEBAR);
		num_clocks = 0;//reset clock ps2 count
		printf("%d\n", ps2_data);
	}
	IO0IntClr |= (1<<0);//clear interrupt
}

int main(void) {

	IO0IntEnF = (1<<0);//enable interrupts on falling edge of P0.0 (Pin 9)
	ISER0 |= (1<<21);//enable interrupts from EINT3 (GPIO)
	FIO0DIR = (1<<6);//set 0.18 (pin 11) as output

    while(1) {
    	FIO0PIN0 = (space_pressed<<6);
    }
    return 0 ;
}

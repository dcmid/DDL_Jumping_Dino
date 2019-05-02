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

#define FIO0DIR0 (*(volatile unsigned int*)0x2009C000) //pin direction reg
#define PINMODE0 (*(volatile unsigned int*)0x4002C040) //pin resistor mode reg
#define FIO0PIN0 (*(volatile unsigned int*)0x2009C014) //pin value reg

#define FIO2DIR0 (*(volatile unsigned int*)0x2009C040)
#define FIO2PIN0 (*(volatile unsigned int*)0x2009C054)
#define PINMODE4 (*(volatile unsigned int*)0x4002C050)

#define FIO2DIR1 (*(volatile unsigned int*)0x2009C041)
#define FIO2PIN1 (*(volatile unsigned int*)0x2009C055)

#define IO0IntEnF (*(volatile unsigned int*)0x40028094)
#define IO0IntClr (*(volatile unsigned int*)0x4002808C)
#define ISER0 (*(volatile unsigned int*)0xE000E100)

#define SCK ((FIO0PIN0 >> 0) & 1) //SCK (ps2 clock) is at P0.0 (pin 9)
#define SDA ((FIO0PIN0 >> 1) & 1) //SDA (ps2 data) is at P0.1 (pin 10)

#define SPACEBAR 0b01001010001


int num_clocks = 0;
int space_pressed = 0;
unsigned int ps2_data = 0;

void wait_ticks(int ticks){
	for(int i =0; i<ticks; i++){}
}

void write_lcd(unsigned int output, int is_data){
	//check if LCD busy
	FIO2DIR0 &= ~(0xFF); //set 2.0-2.7 as inputs
	PINMODE4 |= 0xFF; //set 2.0-2.7 to have pull-down
	FIO2PIN1 &= ~(1<<0); //clear RS
	FIO2PIN1 |= 0b1100; //set RW and E
	wait_ticks(1);
	while(FIO2PIN0 & (1<<7)){//wait for busy signal to clear
		FIO2PIN1 &= ~(1<<3); // clear E (2.11)
		wait_ticks(1);
		FIO2PIN1 |= (1<<3); //set E;
		wait_ticks(1);
	}

	FIO2PIN1 = (is_data && 1); //clear RW and E, set/clear RS if write is data/instruction
	wait_ticks(1);
	FIO2PIN1 |= (1<<3); //set E
	PINMODE4 = 0; //reset pin modes
	FIO2DIR0 |= 0xFF; //set 2.0-2.7 as output
	FIO2PIN0 &= ~(0xFF); //clear outputs
	FIO2PIN0 |= output & 0xFF; //set outputs
	FIO2PIN1 &= ~(1<<3); // clear E (2.11)
}

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
	FIO0DIR0 |= (1<<6);//set 0.6 (pin 8) as output

	FIO2DIR1 |= (0b1101); //set 2.8,2.10,2.11 as outputs
	FIO2PIN1 &= ~(0xFF); //clear control signals to LCD

	write_lcd(0x01, 0);//clear screen
	write_lcd(0x0E, 0);
	write_lcd(0x41, 1);
	write_lcd(0x42, 1);

    while(1) {
    	FIO0PIN0 = (space_pressed<<6);
    }
    return 0 ;
}

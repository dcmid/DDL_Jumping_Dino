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
#include <stdlib.h>

#define FIO0DIR0 (*(volatile unsigned int*)0x2009C000) //pin direction reg
#define PINMODE0 (*(volatile unsigned int*)0x4002C040) //pin resistor mode reg
#define FIO0PIN0 (*(volatile unsigned int*)0x2009C014) //pin value reg

#define FIO2DIR0 (*(volatile unsigned int*)0x2009C040)
#define FIO2PIN0 (*(volatile unsigned int*)0x2009C054)
#define PINMODE4 (*(volatile unsigned int*)0x4002C050)

#define FIO2DIR1 (*(volatile unsigned int*)0x2009C041)
#define FIO2PIN1 (*(volatile unsigned int*)0x2009C055)

#define IO0IntEnF (*(volatile unsigned int*)0x40028094)
#define IO0IntClr (*(volatile unsigned int*)0x4002808C) //used to clear IO interrupts
#define ISER0 (*(volatile unsigned int*)0xE000E100)

#define T0IR (*(volatile unsigned int*)0x40004000) //used to clear Timer0 interrupts
#define T0TCR (*(volatile unsigned int*)0x40004004) //timer control reg
#define T0MCR (*(volatile unsigned int*)0x40004014) //match control reg
#define T0PR (*(volatile unsigned int*)0x4000400C) //prescalar reg
#define T0MR0 (*(volatile unsigned int*)0x40004018) //match reg 0
#define T0TC (*(volatile unsigned int*)0x40004008) //timer counter0

#define SCK ((FIO0PIN0 >> 0) & 1) //SCK (ps2 clock) is at P0.0 (pin 9)
#define SDA ((FIO0PIN0 >> 1) & 1) //SDA (ps2 data) is at P0.1 (pin 10)

#define SPACEBAR 0b01001010001
#define HEAD 0x00
#define FRONTGND 0x01
#define FRONTAIR 0x02
#define BACKGND 0x03
#define BACKAIR 0x04
#define CACTUS 0x05


int num_clocks = 0;
int space_pressed = 0;
unsigned int ps2_data = 0;

int dino_jumping = 0;

void wait_ticks(int ticks){
	for(int i =0; i<ticks; i++){}
}

void write_lcd(unsigned int output, int is_data){
	//check if LCD busy
	FIO2DIR0 &= ~(0xFF); //set 2.0-2.7 as inputs
	PINMODE4 |= 0xFF; //set 2.0-2.7 to have pull-down
	FIO2PIN1 &= ~(1<<0); //clear RS
	FIO2PIN1 |= (1<<2); //set RW
	wait_ticks(1);
	FIO2PIN1 |= (1<<3); // and E
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

void draw(unsigned int chars_ascii[4][20]){
	write_lcd(0x80, 0); //set DDRAM addr to 0
	for(int i=0; i<2; i++){
		for(int j=0; j<20; j++){
			write_lcd(chars_ascii[2*i][j], 1); //draw 1st and 3rd lines
		}
	}

	write_lcd(0xC0, 0); //set DDRAM addr to 0x40 (2nd line according to data sheet)
	for(int i=0; i<2; i++){
		for(int j=0; j<20; j++){
			write_lcd(chars_ascii[2*(i+1)-1][j], 1); //draw 2nd and 4th lines
		}
	}
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

void TIMER0_IRQHandler(void){
	printf("J\n");
	T0IR |= (1<<0); //clear interrupt
}

void shift_chars_ascii(unsigned int chars_ascii[4][20]){
	for(int i=0; i<4; i++){
//		chars_ascii[1][i] = chars_ascii[1][i+1];
//		chars_ascii[2][i] = chars_ascii[2][i+1];
//		chars_ascii[3][i] = chars_ascii[3][i+1];
		if(!(chars_ascii[3][i] == BACKGND || chars_ascii[3][i] == FRONTGND || chars_ascii[3][i+1] == BACKGND))//don't shift dino
			chars_ascii[3][i] = chars_ascii[3][i+1];
	}
	for(int i=4; i<19; i++)
		chars_ascii[3][i] = chars_ascii[3][i+1];
//	chars_ascii[3][0] = 0x5F;
//	chars_ascii[3][1] = 0x5F;
//	chars_ascii[3][2] = BACKGND;
//	chars_ascii[3][3] = FRONTGND;
//	chars_ascii[2][0] = 0x10;
//	chars_ascii[2][1] = 0x10;
//	chars_ascii[2][2] = 0x10;
//	chars_ascii[2][3] = HEAD;
}

int main(void) {
	int dinohead[8] = {0x00, 0x00, 0x00, 0x00, 0x1F, 0x17, 0x1F, 0x18};
	int dinofrontGND[8] = {0x1F, 0x1C, 0x1E, 0x1A, 0x08, 0x08, 0x0C, 0x1F};
	int dinofrontAir[8] = {0x1F, 0x1C, 0x1E, 0x1A, 0x08, 0x08, 0x0C, 0x00};
	int dinobackGND[8] = {0x09, 0x0F, 0x0F, 0x07, 0x02, 0x02, 0x03, 0x1F};
	int dinobackAir[8] = {0x09, 0x0F, 0x0F, 0x07, 0x02, 0x02, 0x03, 0x00};
	int cactus[8] = {0x00, 0x04, 0x15, 0x1F, 0x04, 0x04, 0x04, 0x1F};

	unsigned int chars_ascii[4][20] = {
			{0x10,0x10,0x10,0x44,0x49,0x4E,0x4F,0x10,0x4A,0x55,0x4D,0x50,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10},
			{0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10},
			{0x10,0x10,0x10,HEAD,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10},
			{0x5F,0x5F,BACKGND,FRONTGND,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F},
	};


	IO0IntEnF = (1<<0);//enable interrupts on falling edge of P0.0 (Pin 9)
	ISER0 |= (1<<21);//enable interrupts from EINT3 (GPIO)
	FIO0DIR0 |= (1<<6);//set 0.6 (pin 8) as output

	ISER0 |= (1<<1);//enable interrupts from Timer0
	T0PR = 500; //set prescalar
	T0MR0 = 1000; //set match value
	T0MCR |= (0b11<<0); //enable interrupt on MR0, reset counter on match
	T0TC = 0;
	T0TCR |= (1<<0); //enable Timer0

	FIO2DIR1 |= (0b1101); //set 2.8,2.10,2.11 as outputs
	FIO2PIN1 &= ~(0xFF); //clear control signals to LCD

	write_lcd(0x38, 0);//set 2 line mode COMMENTING THIS OUT MAKES DISPLAY WORK, BUT NOT ALL LINES

	//initialize custom characters
	write_lcd(0x40, 0);
	for(int i=0; i<8; i++) { //create the dino's head
		write_lcd(dinohead[i], 1);
	}
	for(int i=0; i<8; i++) { //create the dino's head
			write_lcd(dinofrontGND[i], 1);
	}
	for(int i=0; i<8; i++) { //create the dino's head
				write_lcd(dinofrontAir[i], 1);
	}
	for(int i=0; i<8; i++) { //create the dino's head
				write_lcd(dinobackGND[i], 1);
	}
	for(int i=0; i<8; i++) { //create the dino's head
				write_lcd(dinobackAir[i], 1);
	}
	for(int i=0; i<8; i++) { //create the dino's head
				write_lcd(cactus[i], 1);
	}

	write_lcd(0x0C, 0);//turn on display and off cursor
	write_lcd(0x01, 0);//clear screen
	//write_lcd(0xC0, 0);//set DDRAM addr to line 2
	draw(chars_ascii);

	//int jump_time

    while(1) {
    	wait_ticks(100000);
    	shift_chars_ascii(chars_ascii);
    	chars_ascii[3][19] = (rand() < RAND_MAX/4) ? CACTUS : 0x5F;
    	draw(chars_ascii);
    	FIO0PIN0 = (space_pressed<<6);
    }
    return 0 ;
}

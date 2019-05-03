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
#include <math.h>
#include <string.h>
#include "dino_jump.h"


int num_clocks = 0;
int space_pressed = 0;
unsigned int ps2_data = 0;

int high_score = 0;

enum g_state {init, play, game_over, reset};

enum g_state game_state = init;

enum d_state {running, jump, fall, djump0, djump1, dfall};

enum d_state dino_state = running;

unsigned int sin_659Hz[6] = {0,0,0,0,0,0};
unsigned int sin_523Hz[23] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
double sound_amp = 0;

const unsigned int INIT_SCREEN[4][20] = {
		{0x10,0x10,0x10,0x44,0x49,0x4E,0x4F,0x10,0x4A,0x55,0x4D,0x50,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10},
		{0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10},
		{0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10},
		{0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F}
};

//initialize array for LCD output
unsigned int chars_ascii[4][20] = {
		{0x10,0x10,0x10,0x44,0x49,0x4E,0x4F,0x10,0x4A,0x55,0x4D,0x50,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10},
		{0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10},
		{0x10,0x10,0x10,HEAD,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10},
		{0x5F,0x5F,BACKGND,FRONTGND,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F}
};

//initialize array of background
unsigned int background[4][20] = {
		{0x10,0x10,0x10,0x44,0x49,0x4E,0x4F,0x10,0x4A,0x55,0x4D,0x50,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10},
		{0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10},
		{0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10},
		{0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F}
};

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

void reset_screen(void){
	memcpy(background[0], INIT_SCREEN[0], sizeof(INIT_SCREEN[0]));
	memcpy(background[1], INIT_SCREEN[1], sizeof(INIT_SCREEN[1]));
	memcpy(background[2], INIT_SCREEN[2], sizeof(INIT_SCREEN[2]));
	memcpy(background[3], INIT_SCREEN[3], sizeof(INIT_SCREEN[3]));
}

void copy_background(void){
	memcpy(chars_ascii[0], background[0], sizeof(background[0]));
	memcpy(chars_ascii[1], background[1], sizeof(background[0]));
	memcpy(chars_ascii[2], background[2], sizeof(background[2]));
	memcpy(chars_ascii[3], background[3], sizeof(background[3]));
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
		if(ps2_data == SPACEBAR){
			space_pressed = 1;
			if(dino_state == running)
				dino_state = jump;
			else if(dino_state == fall)
				dino_state = djump0;
			if(game_state == init)
				game_state = play;
		}
		else
			space_pressed = 0;
		num_clocks = 0;//reset clock ps2 count
		printf("%d\n", ps2_data);
	}
	IO0IntClr |= (1<<0);//clear interrupt
}

void shift_background(){
	for(int i=0; i<19; i++){
		background[0][i] = background[0][i+1];
		background[3][i] = background[3][i+1];
	}
	background[3][19] = (rand() < RAND_MAX/4) ? CACTUS : 0x5F; //1/4 chance of cactus spawn
}

void TIMER0_IRQHandler(void){

	shift_background();//update background array
	for(int i=0; i<4; i++){
		for(int j=0; j<20; j++)
			chars_ascii[i][j] = background[i][j];//copy background into lcd chars array
	}

	//place dino sprite
	switch(dino_state){
	case running:
		chars_ascii[2][3] = HEAD;
		chars_ascii[3][2] = BACKGND;
		chars_ascii[3][3] = FRONTGND;
	break;
	case jump:
		sound_amp = 1;
		dino_state = fall;
		chars_ascii[1][3] = HEAD;
		chars_ascii[2][2] = BACKAIR;
		chars_ascii[2][3] = FRONTAIR;
	break;
	case fall:
		sound_amp = 0;
		dino_state = running;
		chars_ascii[1][3] = HEAD;
		chars_ascii[2][2] = BACKAIR;
		chars_ascii[2][3] = FRONTAIR;
	break;
	case djump0:
		sound_amp = 0;
		dino_state = djump1;
		chars_ascii[0][3] = HEAD;
		chars_ascii[1][2] = BACKAIR;
		chars_ascii[1][3] = FRONTAIR;
	break;
	case djump1:
		dino_state = dfall;
		chars_ascii[0][3] = HEAD;
		chars_ascii[1][2] = BACKAIR;
		chars_ascii[1][3] = FRONTAIR;
	break;
	case dfall:
		dino_state = running;
		chars_ascii[1][3] = HEAD;
		chars_ascii[2][2] = BACKAIR;
		chars_ascii[2][3] = FRONTAIR;
	break;
	}
	chars_ascii[0][19] = 0x30 + high_score%10;
	chars_ascii[0][18] = 0x30 + (high_score/10)%10;
	chars_ascii[0][17] = 0x30 + (high_score/100)%10;
	chars_ascii[0][16] = 0x30 + (high_score/1000)%10;
	printf("STATE: %d\n", dino_state);

	if((chars_ascii[3][2] == BACKGND) && ((background[3][2] == CACTUS) || (background[3][3] == CACTUS)))
		game_state = game_over;

	//state machine for game states (init, play, game over, reset)
	switch(game_state){
	case init://in init, reset game
		srand(T0TC);
		high_score = 0;
		reset_screen();
		copy_background();
		chars_ascii[2][3] = HEAD;
		chars_ascii[3][2] = BACKGND;
		chars_ascii[3][3] = FRONTGND;
		chars_ascii[0][19] = 0x30 + high_score%10;
		chars_ascii[0][18] = 0x30 + (high_score/10)%10;
		chars_ascii[0][17] = 0x30 + (high_score/100)%10;
		chars_ascii[0][16] = 0x30 + (high_score/1000)%10;
		sound_amp = 0;
		if(space_pressed)
			game_state = play;
		draw(chars_ascii);
	break;
	case play://draw screen and increment score
		draw(chars_ascii);
		high_score ++;//increment high_score
	break;
	case game_over://turn on game over sound
		sound_amp = 1;
		game_state = reset;
	break;
	case reset://turn off game over sound
		sound_amp = 0;
		game_state = init;
	break;
	}

	T0IR |= (1<<0); //clear interrupt
}

int main(void) {
	//create sinusoid array
  	for(int i=0; i<6; i++)
		sin_659Hz[i] = (unsigned int)(1024/2*(sin(2*M_PI*659.0*(i/4000.))+1));
	for(int i=0; i<23; i++)
		sin_523Hz[i] = (unsigned int)(1024/2*(sin(2*M_PI*523.0*(i/4000.))+1));


	//GPIO interrupt
	IO0IntEnF = (1<<0);//enable interrupts on falling edge of P0.0 (Pin 9)
	ISER0 |= (1<<21);//enable interrupts from EINT3 (GPIO)
	FIO0DIR0 |= (1<<6);//set 0.6 (pin 8) as output

	//Timer0 interrupt
	ISER0 |= (1<<1);//enable interrupts from Timer0
	T0PR = 500; //set prescalar
	T0MR0 = 1000; //set match value
	T0MCR |= (0b11<<0); //enable interrupt on MR0, reset counter on match
	T0TC = 0;
	T0TCR |= (1<<0); //enable Timer0

	//DAC
	PINSEL1 = (1<<21);
    DACCNTVAL = 250; //set counter so that output is requested at ~4KHz
    DACCTRL |= (1<<2); //set CNT_ENA
    FIO0DIR3 |= 1<<2; //set P0.26 as output (AOUT)
    DACR = 0; //reset DAC output

	FIO2DIR1 |= (0b1101); //set 2.8,2.10,2.11 as outputs
	FIO2PIN1 &= ~(0xFF); //clear control signals to LCD

	write_lcd(0x38, 0);//set 2 line mode COMMENTING THIS OUT MAKES DISPLAY WORK, BUT NOT ALL LINES

	//custom characters
	int dinohead[8] = {0x00, 0x00, 0x00, 0x00, 0x1F, 0x17, 0x1F, 0x18};
	int dinofrontGND[8] = {0x1F, 0x1C, 0x1E, 0x1A, 0x08, 0x08, 0x0C, 0x1F};
	int dinofrontAir[8] = {0x1F, 0x1C, 0x1E, 0x1A, 0x08, 0x08, 0x0C, 0x00};
	int dinobackGND[8] = {0x09, 0x0F, 0x0F, 0x07, 0x02, 0x02, 0x03, 0x1F};
	int dinobackAir[8] = {0x09, 0x0F, 0x0F, 0x07, 0x02, 0x02, 0x03, 0x00};
	int cactus[8] = {0x00, 0x04, 0x15, 0x1F, 0x04, 0x04, 0x04, 0x1F};
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

	int sound_sample = 0;

    while(1) {
    	FIO0PIN0 = (space_pressed<<6);

		if(DACCTRL & (1<<0)){
			if(game_state == reset){
				DACR = (((unsigned int)(sound_amp*sin_523Hz[sound_sample])) << 6);
				sound_sample = (sound_sample+1)%23;
			}
			else{
				DACR = (((unsigned int)(sound_amp*sin_659Hz[sound_sample])) << 6);
				sound_sample = (sound_sample+1)%6;
			}
		}
    }
    return 0 ;
}

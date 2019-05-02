/*
 * dino_jump.h
 *
 *  Created on: May 2, 2019
 *      Author: midk8688
 */

#ifndef DINO_JUMP_H_
#define DINO_JUMP_H_


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


#endif /* DINO_JUMP_H_ */

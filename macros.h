/*
 * macros.h
 *
 *  Created on: 10.08.2017
 *      Author: rauser
 */

#ifndef AVR_MACROS_H_
#define AVR_MACROS_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/delay.h>

#define ADD_ONE_BETWEEN( value, low, high ) \
	( ((value) == (high)) ? ((value) = (low)) : ((value)++) )

#define DEC_ONE_BETWEEN( value, low, high ) \
	( ((value) == (low)) ? ((value) = (high)) : ((value)--) )

#define GPIO_init( port, pin, dir )                  \
	do {                                             \
		if((dir) == INPUT) (port) &= ~(1 << (pin)); \
		else               (port) |=  (1 << (pin)); \
	} while(0)

#define GPIO_set( port, pin, value ) \
	do {                             \
		if ((value) == ON) GPIO_on;  \
		else               GPIO_off; \
	} while(0)

#define GPIO_on( port, pin )    ( (port) |=  (1 << (pin)) )
#define GPIO_off( port, pin )   ( (port) &= ~(1 << (pin)) )
#define GPIO_read( port, pin )  ( (port) &   (1 << (pin)) )
#define GPIO_toggle( port, pin) ( (port) ^=  (1 << (pin)) )

#define GPIO_interrupt( port, pin, interrupt, edgeType )      \
	do {                                                      \
		(port) |= (1 << (pin));       /* Enable pull-up */   \
		GIMSK   |= (1 << (interrupt)); /* Enable interrupt */ \
		MCUCR   |= (edgeType);                                \
		sei();                         /* Enable interrupts */\
	} while(0)

#define PWM_enable( mode, clk, initial )                  \
		do {                                              \
		TCCR0A = (mode);  	/* PWM mode */                \
		TCCR0B = (clk);     /* clock source */            \
		OCR0A  = (initial); /* initial PWM pulse width */ \
		} while(0)

#define PWM_set( value ) ( OCR0A  = (value) )

/*TODO*/
#define PWM_disbale() \
		do {          \
		}while(0)

#define init_random(seed) srand( (seed) )
#define get_random()      rand()
#define get_random_between( min, max ) ( (min) + ( get_random() % ( (max) - (min) ) ) )

#endif /* AVR_MACROS_H_ */

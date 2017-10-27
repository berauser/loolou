//#include "PeripheralNames.h"


#include "defines.h"
#include "macros.h"
#include <stdlib.h>

// ********************************************************
// PWM values
// ********************************************************
#define PWM_OFF   0 // 0V ( OFF )
#define PWM_3V   85 // 3V ( (9V / 255) * 85 = 3V )
#define PWM_9V  255 // 9V

// ********************************************************
// Pin description
// ********************************************************
//
//				  attiny 2313
//           ---------------------
//  RESET   -| 1 (PA2)  (VCC) 20 |- VCC
//  LED5    -| 2 (PD0)  (PB7) 19 |- SCK
//  LED6    -| 3 (PD1)  (PB6) 18 |- MISO
//  LED7    -| 4 (PA1)  (PB5) 17 |- MOSI
//  LED8    -| 5 (PA0)  (PB4) 16 |- FORWARD
//  BUTTON  -| 6 (PD2)  (PB3) 15 |- BACKWARD
//  NC      -| 7 (PD3)  (PB2) 14 |- PWM
//  LED1    -| 8 (PD4)  (PB1) 13 |- LED4
//  LED2    -| 9 (PD5)  (PB0) 12 |- LED3
//  GND     -| 10(GND)  (PD6) 11 |- NC
//           --------------------
//

// *** LEDs ***********************************************
#define PIN_LED1 PD4
#define PIN_LED2 PD5
#define PIN_LED3 PB0
#define PIN_LED4 PB1
#define PIN_LED5 PD0
#define PIN_LED6 PD1
#define PIN_LED7 PA1
#define PIN_LED8 PA0

#define PORT_LED1 PORTD
#define PORT_LED2 PORTD
#define PORT_LED3 PORTB
#define PORT_LED4 PORTB
#define PORT_LED5 PORTD
#define PORT_LED6 PORTD
#define PORT_LED7 PORTA
#define PORT_LED8 PORTA

#define DDR_LED1 DDRD
#define DDR_LED2 DDRD
#define DDR_LED3 DDRB
#define DDR_LED4 DDRB
#define DDR_LED5 DDRD
#define DDR_LED6 DDRD
#define DDR_LED7 DDRA
#define DDR_LED8 DDRA

// *** DIRECTION / PWM ************************************
#define PIN_DIRECTION_FORWARD  PB4
#define PIN_DIRECTION_BACKWARD PB3
#define PIN_PWM    			   PB2

#define PORT_DIRECTION_FORWARD  PORTB
#define PORT_DIRECTION_BACKWARD PORTB
//#define PORT_PWM

#define DDR_DIRECTION_FORWARD  DDRB
#define DDR_DIRECTION_BACKWARD DDRB
#define DDR_PWM                DDRB

#define REGISTER_PWM  OCR0A
#define MODE_PWM      (1 << COM0A1) | (1 << WGM00)
#define CLOCK_PWM     (1 << CS01)

// *** BUTTON *********************************************
#define PIN_BUTTON  PD2
#define PORT_BUTTON PORTD
#define DDR_BUTTON  DDRD
#define EDGE_TYPE_BUTTON (1 << ISC01)

// ********************************************************
// Global variables
// ********************************************************
static GameDirection direction   = FORWARD;
static LEDNumeration active_led  = LED1;
static uint8_t       motor_power = PWM_3V;
static Mode          mode        = M_NORMAL_FORWARD;

// ********************************************************
// Function definition
// ********************************************************
void setup ( void );
void init  ( void );

void show_mode( void );
void show_direction ( void );
void trigger_direction( void );

uint8_t calc_speed( void );
GameDirection calc_direction( void );

// ****************************************************************************
// interrupt service routine
// ***************************************************************************/
ISR(INT0_vect)
{
	ADD_ONE_BETWEEN( mode, M_NORMAL_FORWARD, M_RANDOM_RANDOM );
	show_mode();
}

// ****************************************************************************
// setup system, set pin directions
// ***************************************************************************/
void setup( void )
{
	// leds
	GPIO_init( DDR_LED5, PIN_LED5, OUTPUT ); // 1
	GPIO_init( DDR_LED6, PIN_LED6, OUTPUT ); // 2
	GPIO_init( DDR_LED7, PIN_LED7, OUTPUT ); // 3
	GPIO_init( DDR_LED8, PIN_LED8, OUTPUT ); // 4

	GPIO_init( DDR_LED1, PIN_LED1, OUTPUT ); // red
	GPIO_init( DDR_LED2, PIN_LED2, OUTPUT ); // blue
	GPIO_init( DDR_LED3, PIN_LED3, OUTPUT ); // green
	GPIO_init( DDR_LED4, PIN_LED4, OUTPUT ); // yellow

	// motor control
	GPIO_init( DDR_DIRECTION_BACKWARD, PIN_DIRECTION_BACKWARD, OUTPUT ); // backward
	GPIO_init( DDR_DIRECTION_FORWARD,  PIN_DIRECTION_FORWARD,  OUTPUT ); // forward
	GPIO_init( DDR_PWM, PIN_PWM, OUTPUT ); // PWM
	PWM_enable( MODE_PWM, CLOCK_PWM, REGISTER_PWM, PWM_OFF );

	// button
	GPIO_init( DDR_BUTTON, PIN_BUTTON, INPUT);
	GPIO_interrupt( PORT_BUTTON, PIN_BUTTON, INT0, EDGE_TYPE_BUTTON );
}

// ****************************************************************************
// initialze syetem with default values
// ***************************************************************************/
void init( void )
{
	// set initial values
	direction   = FORWARD;
	active_led  = LED1;
	motor_power = PWM_3V;
	mode        = M_NORMAL_FORWARD;

	// show current mode
	show_mode();

	// initial pwm
	PWM_set( REGISTER_PWM, PWM_3V );

	// set seed
	init_random( TCNT1L );
}

// ****************************************************************************
// shows the current mode
// ***************************************************************************/
void show_mode( void ) {

	switch ( mode )
	{
		case M_NORMAL_FORWARD:
			GPIO_off( PORT_LED5, PIN_LED5 );
			GPIO_off( PORT_LED6, PIN_LED6 );
			GPIO_off( PORT_LED7, PIN_LED7 );
			GPIO_on ( PORT_LED8, PIN_LED8 );
			break;
		case M_FAST_FORWARD:
			GPIO_off( PORT_LED5, PIN_LED5 );
			GPIO_off( PORT_LED6, PIN_LED6 );
			GPIO_on ( PORT_LED7, PIN_LED7 );
			GPIO_on ( PORT_LED8, PIN_LED8 );
			break;
		case M_RANDOM_FORWARD:
			GPIO_off( PORT_LED5, PIN_LED5 );
			GPIO_on ( PORT_LED6, PIN_LED6 );
			GPIO_on ( PORT_LED7, PIN_LED7 );
			GPIO_on ( PORT_LED8, PIN_LED8 );
			break;
		case M_RANDOM_RANDOM:
			GPIO_on ( PORT_LED5, PIN_LED5 );
			GPIO_on ( PORT_LED6, PIN_LED6 );
			GPIO_on ( PORT_LED7, PIN_LED7 );
			GPIO_on ( PORT_LED8, PIN_LED8 );
			break;
		default: /* unkown */
			GPIO_off( PORT_LED5, PIN_LED5 );
			GPIO_off( PORT_LED6, PIN_LED6 );
			GPIO_off( PORT_LED7, PIN_LED7 );
			GPIO_off( PORT_LED8, PIN_LED8 );
			break;
	}
}

// ****************************************************************************
// shows the current direction
// ***************************************************************************/
void show_direction ( void )
{
	if( direction == FORWARD )	ADD_ONE_BETWEEN( active_led, LED1, LED4 );
	else                        DEC_ONE_BETWEEN( active_led, LED1, LED4 );

	switch ( active_led )
	{
		case LED1:
			GPIO_off( PORT_LED1, PIN_LED1 );
			GPIO_off( PORT_LED2, PIN_LED2 );
			GPIO_off( PORT_LED3, PIN_LED3 );
			GPIO_on ( PORT_LED4, PIN_LED4 );
			break;
		case LED2:
			GPIO_off( PORT_LED1, PIN_LED1 );
			GPIO_off( PORT_LED2, PIN_LED2 );
			GPIO_on ( PORT_LED3, PIN_LED3 );
			GPIO_off( PORT_LED4, PIN_LED4 );
			break;
		case LED3:
			GPIO_off( PORT_LED1, PIN_LED1 );
			GPIO_on ( PORT_LED2, PIN_LED2 );
			GPIO_off( PORT_LED3, PIN_LED3 );
			GPIO_off( PORT_LED4, PIN_LED4 );
			break;
		case LED4:
			GPIO_on ( PORT_LED1, PIN_LED1 );
			GPIO_off( PORT_LED2, PIN_LED2 );
			GPIO_off( PORT_LED3, PIN_LED3 );
			GPIO_off( PORT_LED4, PIN_LED4 );
			break;
		default: /* unkown */
			GPIO_off( PORT_LED1, PIN_LED1 );
			GPIO_off( PORT_LED2, PIN_LED2 );
			GPIO_off( PORT_LED3, PIN_LED3 );
			GPIO_off( PORT_LED4, PIN_LED4 );
			break;
	}
}

// ****************************************************************************
// change the direction
// ***************************************************************************/
void trigger_direction( void )
{
	if( direction == FORWARD )
	{
		GPIO_on ( PORT_DIRECTION_FORWARD,  PIN_DIRECTION_FORWARD  );
		GPIO_off( PORT_DIRECTION_BACKWARD, PIN_DIRECTION_BACKWARD );
	}
	else
	{	/* BACKWARD */
		GPIO_off( PORT_DIRECTION_FORWARD,  PIN_DIRECTION_FORWARD  );
		GPIO_on ( PORT_DIRECTION_BACKWARD, PIN_DIRECTION_BACKWARD );
	}
}

// ****************************************************************************
// calculates a new direction
// ***************************************************************************/
GameDirection calc_direction( void )
{
	switch( mode )
	{
		case M_NORMAL_FORWARD:
		case M_FAST_FORWARD:
		case M_RANDOM_FORWARD:
		default:
			return FORWARD;
			break;
		case M_RANDOM_RANDOM:
			return ( (get_random_between( 0, 10 ) == 0) ? BACKWARD : FORWARD ); /* 10% backward : 90% forward */
			break;
	}
}

// ****************************************************************************
// calculates a new speed
// ***************************************************************************/
uint8_t calc_speed( void )
{
	switch( mode )
	{
	case M_NORMAL_FORWARD:
	default:
		return PWM_3V;
		break;
	case M_FAST_FORWARD:
		return PWM_9V;
		break;
	case M_RANDOM_FORWARD:
	case M_RANDOM_RANDOM:
	{
		/*
		 * Calculate speed in 25 steps.
		 * 0 = 0V, ...,  25 = ~9V
		 *
		 * For a better and faster playing pleasure,
		 * a probability of
		 * 10% values between   0V and 3,5V and
		 * 70% values between 3,5V and 6,0V is selected.
		 * 20% values between 7,0V and 9,0V is selected.
		 */
		static uint8_t r = 0;
		r = get_random_between( 0, 10 );
		if ( r == 0 )
		{ /* 20% slow ( 0V - ~3,5V ) */
			return get_random_between(  0, 100 );
		}
		else if ( r == 1 || r == 2 )
		{ /* 20% very fast /~7,0V - 9,0V */
			return get_random_between( 201, 255);
		}
		else
		{ /* 80% fast ( ~3,5V - ~7,0V ) */
			return get_random_between( 101, 200 );
		}
	}
		break;

	}
}

// ****************************************************************************
// game
// ***************************************************************************/
void play( void )
{
	static uint8_t cycle = 0;
	if( cycle == 0 )
	{ /* is called every 1 second */
		direction = calc_direction();
		trigger_direction();
		PWM_set( REGISTER_PWM, calc_speed() );
	}
	show_direction();
	ADD_ONE_BETWEEN( cycle, 0, 10 );
	_delay_ms( 100 );
}

int main(void) {

	setup();
	init();

	/* main loop */
	while (1) {
		play();
	}

	return 0;
}

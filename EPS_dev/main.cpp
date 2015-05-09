/*
 * main.cpp
 *
 *  Created on: May 9, 2015
 *      Author: quentin
 */

#include <msp430.h>

/******************************/
//alias definition for the pins
//analogic onboard current and voltage sensors
#define SOL_CURRENT
#define BAT_CURRENT
#define BUS_VOLT
#define BAT_VOLT
//protected external power switches
#define EXT_PW1
#define EXT_PW2
#define EXT_PW3
//heater on Hexfet transistors
#define HEAT1
#define HEAT2
#define HEAT3
//4 external analog inputs
#define TEMP1
#define TEMP2
#define TEMP3
#define TEMP4
//enable pin for battery charging
#define EN_CHG
//I2C address for mainboarf
#define I2C_MAIN_ADDR

/********************************/
// state machine
#define PARAM_STATUS  0  //store status on one byte

	#define FLAG_EMERGENCY  0     //main is OFF
	#define FLAG_COM        1     //COM power status
	#define FLAG_MOTORS     2     //MOTORS power status
	#define FLAG_HEAT1      3
	#define FLAG_HEAT2      4
	#define FLAG_HEAT3      5
	#define FLAG_CHARGING   6
	#define FLAG_TEMP_BT_OK 7

/*******************************/
//errors to be solved by Main
#define PARAM_ERROR_BYTE
	#define ERROR_MOTOR_STALL

/******************************/
//Transmitted parameters
#define PARAM_POWER_CONSO
#define PARAM_POWER_PROD
#define PARAM_ESTIMATED_AUTONOMY
#define PARAM_BAT_VOLT
#define PARAM_BUS_VOLT
#define PARAM_BAT_TEMP


/*
 * main.c
 */
void main(void) {

	while(1){
		WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	}

}



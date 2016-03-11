/*
 * module_control.h
 *
 *  Created on: 08 Mar 2016
 *      Author: raffael
 */

#ifndef MODULE_CONTROL_H_
#define MODULE_CONTROL_H

#include "commands.h"

#define N_MODULES	4
//indexes:
#define M331			0
#define M332			1
#define M5			2
#define M11			3


#define ON		1
#define OFF		0, COMM_OK // turning a module off is always allowed
#define MAIN_ON	0, COMM_OK // turning main module on is always allowed
#define MAIN_OFF 1, COMM_OK // active low.

#define PORT_3V3_M_EN		&P1OUT
#define PIN_3V3_M_EN			BIT3

#define PORT_3V3_1_EN		&P1OUT
#define PIN_3V3_1_EN			BIT4

#define PORT_3V3_2_EN		&P1OUT
#define PIN_3V3_2_EN			BIT5


#define PORT_5V_EN			&P1OUT
#define PIN_5V_EN			BIT6

#define PORT_11V_EN			&P1OUT
#define PIN_11V_EN			BIT7

void module_control(volatile unsigned char * port, unsigned char pin, char state, char permission);
#endif /* MODULE_CONTROL_H_ */

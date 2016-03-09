/*
 * commands.h
 *
 *  Created on: 08 Mar 2016
 *      Author: raffael
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_

// Switch modules on / off
#define M3V3_1_ON	0x31
#define M3V3_2_ON	0xD1
#define M3V3_1_OFF	0x30
#define M3V3_2_OFF	0xD0
#define M11V_ON		0xB1
#define M5V_ON		0x51
#define M11V_OFF	0xB0
#define M5V_OFF		0x50

#define SC_ON		0xC1
#define SC_OFF		0xC0

// Confirmations (EPS responses)
#define OK					0x1A
#define LOW_VOLTAGE			0x99
#define UNKNOWN_COMMAND		0x77
#define ERROR				0xFA

// Ask for status data
#define V_BAT		0x0B
#define V_SC			0x0C
#define I_IN			0x11
#define I_OUT		0x10





#endif /* COMMANDS_H_ */

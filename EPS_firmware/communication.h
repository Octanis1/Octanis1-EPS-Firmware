/*
 * communication.h
 *
 *  Created on: 11 Mar 2016
 *      Author: raffael
 */

#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

#include <stdint.h>
#include "module_control.h"

typedef struct _eps_status { //stores the answers to be sent to an eventual i2c request
	uint16_t v_bat;
	uint16_t v_solar;
	uint16_t current_in;
	uint16_t current_out;
	uint16_t analog_ext1;
	uint16_t analog_ext2;
	uint16_t analog_ext3;
	uint16_t analog_ext4;
} eps_status_t;

extern eps_status_t eps_status;
extern uint8_t module_status[N_MODULES]; //stores the answers to be sent to an eventual i2c request


void init_i2c();
void execute_i2c_command();



#endif /* COMMUNICATION_H_ */

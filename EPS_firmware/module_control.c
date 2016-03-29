/*
 * module_control.c
 *
 *  Created on: 08 Mar 2016
 *      Author: raffael
 */

#include "module_control.h"

void module_control(volatile unsigned char * port, unsigned char pin, char state, char permission)
{
	if(permission == COMM_OK)
	{
		if(state)
			*port |= pin;
		else
			*port &= ~pin;
	}
}




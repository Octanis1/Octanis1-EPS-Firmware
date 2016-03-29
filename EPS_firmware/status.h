/*
 * status.h
 *
 *  Created on: 10 Mar 2016
 *      Author: raffael
 */

#ifndef STATUS_H_
#define STATUS_H_
#define ANALOG_6 //for 2/4 external analog inputs (2 digital IO), comment to set all 4 A_EXT as analog IN
#ifdef ANALOG_6 //Array with the power monitoring analog + 2 externals
	#define ANALOG_PORTS 6
#else           //Array with the power monitoring analog + 4 externals
	#define ANALOG_PORTS 8
#endif

#define NB_ANALOG_ACQ 16
//battery voltage threshold levels
#define THRESHOLD_80	810	 	//80% of charge (100% is 4.2V)
#define THRESHOLD_60	761		//60% of charge
#define THRESHOLD_40	712		//40% of charge
#define THRESHOLD_20	663		//20% of charge (0% is 3V)
//battery temperature thresholds in ADC counts
#define COLD_20			385		//too cold for charging
#define COLD_0			672		//ok for charging
#define HOT_30			1020	//too hot, shut lower the PID duty cycle
#define T_BAT_OK		800

void init_adc();

void update_self_status();


#endif /* STATUS_H_ */

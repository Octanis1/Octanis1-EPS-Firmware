/*
 * status.c
 *
 *  Created on: 10 Mar 2016
 *      Author: raffael
 */

#include <msp430.h>
#include "status.h"
#include "module_control.h"
#include "communication.h"


static uint16_t analog_reads[16*ANALOG_PORTS]; //analog readings array
static unsigned int analog_avg[ANALOG_PORTS];	   //avergaed analog readings


enum adc_status_{
	ADC_BUSY, 	//wait for it to finish
	IDLE, 	//not in use, and not to be triggered
	START, 	//the periodic timer interrupt requires a new measurement, therefore start a new ADC conversion
	DONE		//ADC conversion is done, read values to status variable
} adc_status;

void init_adc()
{
	ADC10CTL0 = ADC10SHT_2 + MSC + ADC10ON + ADC10IE + REFON + REF2_5V; // 4*ADC10CLK clycle sample period, Fast auto sequence, enable ADC, IRQ, internal reference ON at 2.5V
	ADC10DTC1 = NB_ANALOG_ACQ*ANALOG_PORTS;   // 16 conversions per channel
	#ifdef ANALOG_6			        //4 analog inputs
	ADC10CTL1 = INCH_5 + CONSEQ_3;  // A3-A0, repeat multi-channel mode
	ADC10AE0 = 0x3F;                // ADC option select (select A0 - A5)
	#else 							//8 analog inputs
	ADC10CTL1 = INCH_7 + CONSEQ_3;  // A3-A0, repeat multi-channel mode
	ADC10AE0 = 0xFF;                // ADC option select (select A0 - A7)
	#endif
}


//initialize a conversion
void trigger_adc()
{
	ADC10CTL0 &= ~ENC;						//disable before enable, safety
	while (ADC10CTL1 & BUSY);               // Wait if ADC10 core is active
	ADC10SA = analog_reads;                 // Data buffer start, pointer to the analog data array
	ADC10CTL0 |= ENC + ADC10SC;             // Enable conversion & Start conversion set up
	adc_status = ADC_BUSY;
}

//determine the average adc value for each analog port
void avg_adc_values()
{
	uint8_t i=0;
	uint8_t j=0;
	for(i=0;i<ANALOG_PORTS;i++){
		analog_avg[i]=0;  //reset the avg table
		for(j=0;j<NB_ANALOG_ACQ;j++){
			//sum up all the acquisitions
			analog_avg[i]=analog_avg[i]+analog_reads[i+ANALOG_PORTS*j];
		}
		analog_avg[i]=analog_avg[i]/10; //averaging
	}
}

//determine the thresholds table
void thd_adc_values(){
	uint8_t i=0;
	for(i=0;i<N_MODULES;i++) 	//everything can be turned ON
		module_status[i]=1;

	if(eps_status.v_bat<THRESHOLD_80){ 	//shut down the motors and extra heaters
		module_status[M11]=0;
		module_status[HT2]=0;
		module_status[HT3]=0;
	}

	if(eps_status.v_bat<THRESHOLD_60){ 	//shut down the 5V and externa 3.3V port # 2
		module_status[M5]=0;
		module_status[M332]=0;
	}

	if(eps_status.v_bat<THRESHOLD_40) 	//shut down the 3.3V #1
		module_status[M331]=0;

	if(eps_status.v_bat<THRESHOLD_20) 	//shut down the battery heater
		module_status[HT1]=0;

}

void read_adc_values()
{
	//power monitoring
	eps_status.current_out=(uint16_t)analog_avg[0];
	eps_status.current_in=(uint16_t)analog_avg[1];
	eps_status.v_solar=(uint16_t)analog_avg[2];
	eps_status.v_bat=(uint16_t)analog_avg[3];
	//external analog readings
	eps_status.analog_ext1=(uint16_t)analog_avg[4];
	eps_status.analog_ext2=(uint16_t)analog_avg[5];
	#ifndef ANALOG_6
		eps_status.analog_ext3=(uint16_t)analog_avg[6];
		eps_status.analog_ext4=(uint16_t)analog_avg[7];
	#endif
}



void update_self_status()
{
	if(adc_status == START)
		trigger_adc();

	if(adc_status == DONE){ // NOTE: no "else if" here, because CPU wakes up from LPM and ADC is done
		avg_adc_values();
		read_adc_values();
		thd_adc_values();
	}
}


#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void)
{
	adc_status = DONE;
	__bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}

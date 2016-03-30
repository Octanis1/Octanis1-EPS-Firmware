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
	IDLE, 	//not in use, and not to be triggered
	ADC_BUSY, 	//wait for it to finish
	START, 	//the periodic timer interrupt requires a new measurement, therefore start a new ADC conversion
	DONE		//ADC conversion is done, read values to status variable
} adc_status;

void init_adc()
{
	adc_status = IDLE;
	ADC10CTL0 = ADC10SHT_1 + MSC + ADC10ON + ADC10IE + REFON + REF2_5V; // 4*ADC10CLK clycle sample period, Fast auto sequence, enable ADC, IRQ, internal reference ON at 2.5V
	ADC10DTC1 = (NB_ANALOG_ACQ*ANALOG_PORTS);   // 16 conversions per channel
	ADC10DTC0=0;
	#ifdef ANALOG_6			        //4 analog inputs
	ADC10CTL1 = INCH_5 + CONSEQ_3;  // A5-A0, repeat multi-channel mode
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
	ADC10SA = (uint16_t)analog_reads;       // Data buffer start, pointer to the analog data array

	ADC10CTL0 |= ENC + ADC10SC;             // Enable conversion & Start conversion set up
	adc_status = ADC_BUSY;
	ADC10CTL0 |= ADC10SC;
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
		analog_avg[i]=analog_avg[i]>>4; //averaging
	}
}

//determine the thresholds table
void thsd_adc_values(){
	uint8_t i=0;
	for(i=0;i<N_MODULES;i++) 	//everything can be turned ON
		module_status[i]=COMM_OK;

	if(eps_status.v_bat<THRESHOLD_80){ 	//shut down the motors and extra heaters
		module_status[M11]=LOW_VOLTAGE;
		module_status[HT2]=LOW_VOLTAGE;
		module_status[HT3]=LOW_VOLTAGE;
	}

	if(eps_status.v_bat<THRESHOLD_60){ 	//shut down the 5V and externa 3.3V port # 2
		module_status[M5]=LOW_VOLTAGE;
		module_status[M332]=LOW_VOLTAGE;
	}

	if(eps_status.v_bat<THRESHOLD_40); 	//shut down the 3.3V #1
//		module_status[M331]=LOW_VOLTAGE;

	if(eps_status.v_bat<THRESHOLD_20); 	//shut down the battery heater
//		module_status[HT1]=LOW_VOLTAGE;
}

uint16_t thsd_battery_temp(unsigned int ext_read){
	if(ext_read<COLD_20)
		return COLD_20;
	if(ext_read<COLD_0)
		return COLD_0;
	if(ext_read>HOT_30)
		return HOT_30;
	else return T_BAT_OK;
}

void read_adc_values()
{
	//power monitoring
	eps_status.current_out=(uint16_t)analog_avg[ANALOG_PORTS-1];
	eps_status.current_in=(uint16_t)analog_avg[ANALOG_PORTS-2];
	eps_status.v_solar=(uint16_t)analog_avg[ANALOG_PORTS-3];
	eps_status.v_bat=(uint16_t)analog_avg[ANALOG_PORTS-4];
	//external analog readings
	eps_status.analog_ext1=(uint16_t)analog_avg[ANALOG_PORTS-5];
	eps_status.t_bat=thsd_battery_temp(analog_avg[ANALOG_PORTS-5]);
	eps_status.analog_ext2=(uint16_t)analog_avg[ANALOG_PORTS-6];
	#ifndef ANALOG_6
		eps_status.analog_ext3=(uint16_t)analog_avg[ANALOG_PORTS-7];
		eps_status.analog_ext4=(uint16_t)analog_avg[ANALOG_PORTS-8];
	#endif

	//power monitoring when only 8 bit comm is implemented for i2c
	eps_status.current_out_8=(uint8_t)(eps_status.current_out);
	eps_status.current_in_8=(uint8_t)(eps_status.current_in);
	eps_status.v_solar_8=(uint8_t)(eps_status.v_solar);
	eps_status.v_bat_8=(uint8_t)(eps_status.v_bat-THRESHOLD_0);
	//external analog readings
	eps_status.analog_ext1_8=(uint8_t)(eps_status.analog_ext1>>2);
	eps_status.t_bat_8=(uint8_t)(eps_status.t_bat>>2);
	eps_status.analog_ext2_8=(uint8_t)(eps_status.analog_ext2>>2);
	#ifndef ANALOG_6
		eps_status.analog_ext3_8=(uint8_t)(eps_status.analog_ext3>>2)];
		eps_status.analog_ext4_8=(uint8_t)(eps_status.analog_ext4>>2);
	#endif
}



void update_self_status()
{
	if(adc_status == IDLE)
		adc_status = START;

	if(adc_status == START)
		trigger_adc();

	if(adc_status == ADC_BUSY)
		__bis_SR_register(CPUOFF + GIE);   // Wait if ADC10 core is active

	if(adc_status == DONE){ // NOTE: no "else if" here, because CPU wakes up from LPM and ADC is done
		avg_adc_values();
		read_adc_values();
		thsd_adc_values();
		adc_status=IDLE;
	}

}


#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void)
{
	adc_status = DONE;
	__bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}


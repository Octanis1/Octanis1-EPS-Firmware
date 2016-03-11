/*
 * status.c
 *
 *  Created on: 10 Mar 2016
 *      Author: raffael
 */

#include <msp430.h>
#include "status.h"

enum adc_status_{
	ADC_BUSY, 	//wait for it to finish
	IDLE, 	//not in use, and not to be triggered
	START, 	//the periodic timer interrupt requires a new measurement, therefore start a new ADC conversion
	DONE		//ADC conversion is done, read values to status variable
} adc_status;

void init_adc()
{
	ADC10CTL1 = INCH_1 + CONSEQ_3;            // A1/A0, repeat multi channel
	ADC10CTL0 = ADC10SHT_2 + MSC + ADC10ON + ADC10IE;
	ADC10AE0 = 0x03;                          // P2.0,1 ADC option select
	ADC10DTC1 = 0x40;                         // 64 conversions
}

void trigger_adc()
{
	ADC10CTL0 &= ~ENC;
	while (ADC10CTL1 & BUSY);               // Wait if ADC10 core is active
	ADC10SA = 0x200;                        // Data buffer start
	ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion ready
	adc_status = ADC_BUSY;
}

void read_adc_values()
{

}


void update_self_status()
{
	if(adc_status == START)
		trigger_adc();

	if(adc_status == DONE) // NOTE: no "else if" here, because CPU wakes up from LPM and ADC is done
		read_adc_values();
}


#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void)
{
	adc_status = DONE;
	__bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}

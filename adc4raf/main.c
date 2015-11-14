//*****************************************************************************
//
// MSP432 main.c template - Empty main
//
//****************************************************************************

#include "msp.h"

void adcInterrupt()
{
    //increment counter
    //sum up results
}

void initADC(void)
{
	//use PySELx to disable digital part of analog pins
	//ADC14ON = 1; //otherwise in reset
	//config ADC

    ADC14CTL0 = /*ADC14PDIV*/   //ADC14 predivider
              + ADC14SHS_0      //sample and hold source ADCSC bit
              + ADC14SHP        //sample and hold timer based on ADCCLK (pulse mode)
            /*+ ADC14DIV*/      //ADC14 divider
            /*+ ADC14SSEL*/     //ADC14 clock source
              + ADC14CONSEQ_2   //repeat single channel conversion TODO maybe scan 4 channels, repeat
            /*+ ADC14SHT1x*/    //sample and hold time MEM 8-23
            /*+ ADC14SHT0x*/    //sample and hold time MEM 0-7 24-31
              + ADC14MSC        //multiple conversions without retriggering
              + ADC14ON;        //turn on ADC

    ADC14CTL1 = (0<<16)         //conversion start register address (0)
              + ADC14RES_2      //12bit resolution
              + ADC14REFBURST   //reference only active during sample-conversion
              + ADC14PWRMD_2;   //low power mode max 12bit @ 200ksps

    ADC14MCTL0 = ADC14VRSEL_1   //buffered Vref and AVss as reference
//               + ADC14EOS       //end of sequence
               + ADC14INCH_20;  //input channel A20

    ADC14MCTL1 = ADC14VRSEL_1   //buffered Vref and AVss as reference
//               + ADC14EOS       //end of sequence
               + ADC14INCH_20;  //input channel A20

    ADC14MCTL2 = ADC14VRSEL_1   //buffered Vref and AVss as reference
//               + ADC14EOS       //end of sequence
               + ADC14INCH_20;  //input channel A20

    ADC14MCTL3 = ADC14VRSEL_1   //buffered Vref and AVss as reference
               + ADC14EOS       //end of sequence
               + ADC14INCH_20;  //input channel A20

}

void main(void)
{
	
    WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer


	
}

/*
 * main.cpp
 *
 *  Created on: Aug, 2015
 *      Author: Quentin Cabrol
 */

#include <msp430.h>
#include <msp430g2744.h>
#include "TI_USCI_I2C_slave.h"

/*********************************************************************/
//I2C callback functions annd address definitions
//http://www.ti.com/general/docs/litabsmultiplefilelist.tsp?literatureNumber=slaa383

#define I2C_EPS_SLAVE_ADDR 0X90

static unsigned char RXData;
static unsigned char TXData;

/* callback for start condition */
void start_cb();

/* callback for bytes received */
void receive_cb(unsigned char receive);

/* callback to transmit bytes */
void transmit_cb(unsigned char volatile *send_next);



/******************************/
//alias definition for the pins
//analogic onboard current and voltage sensors
#define CH_SOL_CURRENT		INCH_2
#define CH_BAT_CURRENT		INCH_1
#define CH_BUS_VOLT			INCH_6
#define CH_BAT_VOLT			INCH_7
//4 external analog inputs
#define ANALOG1		INCH_12
#define ANALOG2 	INCH_13
#define ANALOG3 	INCH_14
#define ANALOG4 	INCH_15
//protected external power switches  --> to be set
#define EXT_PW1		BIT5
#define EXT_PW2		BIT6
#define EXT_PW3		BIT7
//heater on Hexfet transistors   --> to be set
#define HEAT1		BIT1
#define HEAT2		BIT2
#define HEAT3		BIT3
//enable pin for battery charging  --> to be set
#define EN_CHG		BIT4
#define EN_SDOWN    BIT3
//output port
#define CTRL_PORT	P1OUT
//I2C address for mainboarf  --> to be set
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

/*****************************/
//conversion factors and offsets
//Max 2500 mV maeasurement, Voltage divided by 2
#define FACTOR_BAT_VOLT (2535*2)
#define FACTOR_BUS_VOLT (2500*2)
//Max 2500 mV measurement, 10mOhm resistor, gain of 50
#define FACTOR_SOL_CURRENT (2*2500)
#define FACTOR_BAT_CURRENT (2*2500)
#define FACTOR_ANALOG1 (1024)
#define FACTOR_ANALOG2 (1024)
#define FACTOR_ANALOG3 (1024)
#define FACTOR_ANALOG4 (1024)

#define FACTOR_ADC (1024)

#define OFFSET_BAT_VOLT		(0)
#define OFFSET_BUS_VOLT		(39)
#define OFFSET_SOL_CURRENT	(0)
#define OFFSET_BAT_CURRENT	(0)
#define OFFSET_ANALOG1		(0)
#define OFFSET_ANALOG2		(0)
#define OFFSET_ANALOG3		(0)
#define OFFSET_ANALOG4		(0)

long buffer32;

void EPS_init(void){

	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    //initialize clock
    BCSCTL1 &= ~(XTS + DIVA_3); //No prescaler for ACLK, directly lowfreq clock source
    BCSCTL1 |= XT2OFF; //HF XTAL2 off (if available)
    BCSCTL2 = SELM_0 + DIVM_0 + DIVS_0;
    BCSCTL3 = LFXT1S_2; //LFO source for ACLK (about 12 kHz)

    //Switch to 1 MHz from DCO
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    //Make sure still same configuration as before switching to 1 MHz
    BCSCTL1 &= ~(XTS + DIVA_3); //No prescaler for ACLK, directly lowfreq clock source
    BCSCTL1 |= XT2OFF; //HF XTAL2 off (if available)

    //SVSCTL = 0x00; //disable supply voltage supervisor, issue, right name not found
	//initializing output values
	CTRL_PORT=0b11100000; //Ext switches off (inverted), Charging off, all heaters off
	P4OUT=0b00000000; //Keep StepDown enabled (inverted P4.3)
	//initializing pins I/O direction 0 = IN 1=OUT
	P1DIR=0b11111110; //all but P1.0 are OUTPUTS
	P2DIR=0b10000000; //all but XOUT are INPUTS
	P3DIR=0b00000000; //all INPUTs
	P4DIR=0b00001000; //all INPUTs, P4.3 output (stepdown)
	//initializing pins mode 0 = I/O 1=AUX
	P1SEL=0x00;
	P2SEL=0xC6; //init (A01) (A02) and (Xin) (Xout) functions TO BE CHECKED for XIN !!
	P3SEL=0xC6; //init (A06) (A07) and SDA SCL functions
	P4SEL=0x70; //init (A13) to (A15)
	//initializing analog pins
	ADC10AE0 = 0b11000110; //(A01)(A02)(A06)(A07)
	ADC10AE1 = 0b11100000; //(A13)(A14)(A15)

	//init I2C with EPS as slave
	TI_USCI_I2C_slaveinit(start_cb, transmit_cb, receive_cb, I2C_EPS_SLAVE_ADDR); // init the slave
	_EINT();

    //initializing ADC
    ADC10CTL1 = ADC10SSEL_3 + ADC10DIV_0;            // single channel; SMCLK
    ADC10CTL0 = ADC10SHT_0 + SREF_1 + ADC10ON
              + REFON + REF2_5V + REFBURST; //4 clock cycle SH, 2.5V/0V reference, Burst reference

	//initializing ADC
//	ADC10CTL1 = ANALOG4 + CONSEQ_1 + ADC10SSEL_2 + ADC10DIV_7;            // A7 to A0, single sequence
//	ADC10CTL0 = ADC10SHT_1 + SREF_1 + MSC + ADC10ON
//			  /*+ ADC10IE*/ + REFON + REF2_5V + REFBURST; //16 clock cycle SH, 2.5V/0V reference, Burst reference, interrupt enabled
}

int updateMeasurements(int channel) {
        //ADC measurements (automatic update in BG?)
        ADC10CTL0 &= ~ENC;
        while (ADC10CTL1 & BUSY);               // Wait if ADC10 core is active

        //ADC10SA = (unsigned int) buffer;      // Data buffer start
        ADC10CTL1 = channel + ADC10SSEL_3 + ADC10DIV_0;     // A15 to A0, single sequence, SMCLK
        //ADC10DTC1 = 15;                       // 16 conversions

        ADC10CTL0 &= ~(ADC10IFG);
        ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start

        while(!(ADC10CTL0 & ADC10IFG));
        return ADC10MEM;
}


/*
 * main.c
 */
void main(void) {

	char flags = 0x01; // start in emergency state with main off
	//Voltage and current in mV and mA
	unsigned int Vbus = 0;
	unsigned int Vbat = 0;
	unsigned int Ibat = 0;
	unsigned int Isol = 0;

	int Temp1 = 0;
	int Temp2 = 0;
	int Temp3 = 0;
	int Temp4 = 0;

	//int AdcResults[16];

	EPS_init();

//	while(1){

//	}

	for(;;)
	{
		// update voltage and batery values, blocking call to ADC, putting CPU to sleep
		//updateMeasurements(1, AdcResults);

		// conversion raw values to correct values & ref. values
		Ibat = (unsigned int)((long)updateMeasurements(CH_BAT_CURRENT) * (long)FACTOR_BAT_CURRENT / FACTOR_ADC) + OFFSET_BAT_CURRENT;
		Isol = (unsigned int)((long)updateMeasurements(CH_SOL_CURRENT) * (long)FACTOR_SOL_CURRENT / FACTOR_ADC) + OFFSET_SOL_CURRENT;
		Vbus = (unsigned int)((long)updateMeasurements(CH_BUS_VOLT) * (long)FACTOR_BUS_VOLT / FACTOR_ADC) + OFFSET_BUS_VOLT;
		Vbat = (unsigned int)((long)updateMeasurements(CH_BAT_VOLT) * (long)FACTOR_BAT_VOLT / FACTOR_ADC) + OFFSET_BAT_VOLT;

		//Temp1 = (int)((long)AdcResults[12] * (long)FACTOR_ANALOG1 / FACTOR_ADC) + OFFSET_ANALOG1;
		//Temp2 = (int)((long)AdcResults[13] * (long)FACTOR_ANALOG2 / FACTOR_ADC) + OFFSET_ANALOG2;
		//Temp3 = (int)((long)AdcResults[14] * (long)FACTOR_ANALOG3 / FACTOR_ADC) + OFFSET_ANALOG3;
		//Temp4 = (int)((long)AdcResults[15] * (long)FACTOR_ANALOG4 / FACTOR_ADC) + OFFSET_ANALOG4;

		//Enable Mainboard
		if(flags & (1<<FLAG_EMERGENCY))
		{
			if(Vbat > 3200 && Vbus > 3300) {
				CTRL_PORT &= ~(EXT_PW1 + EXT_PW2 + EXT_PW3); //Enable mainboard
				flags &= ~(1<<FLAG_EMERGENCY);
			}
		}
		else {
			if(Vbat < 2800 || Vbus < 3150) {
				CTRL_PORT |= EXT_PW1 + EXT_PW2 + EXT_PW3; //Disable mainboard
				flags |= (1<<FLAG_EMERGENCY);
			}
		}

		//Charging
		if(flags & (1<<FLAG_CHARGING)) {
			if(Vbat > 4200 || Vbus < 3320) {
				CTRL_PORT &= ~EN_CHG; //Disable charging
				P4OUT &= ~EN_SDOWN; //Enable stepdown
				flags &= ~(1<<FLAG_CHARGING);
			}
		}
		else {
			if(Vbat < 4180 && Vbus > 3350) {
                CTRL_PORT |= EN_CHG; //Enable charging
			    P4OUT |= EN_SDOWN; //Disable stepdown
				flags |= (1<<FLAG_CHARGING);
			}
		}
	}
}



void start_cb(){
RXData = 0;
}
void receive_cb(unsigned char receive){
RXData = receive;
}
void transmit_cb(unsigned char volatile *send_next){
*send_next = TXData++;
}

//// USCI_B0 Data ISR
//#pragma vector = ADC10_VECTOR
//__interrupt void ADC10_ISR(void)
//{
//	int i;
//	i = 1;
//	while(i)
//		i = 0;
//	//Interrupt is just used to wake up processor
//}

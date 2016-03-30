/*
 * communication.c
 *
 *  Created on: 11 Mar 2016
 *      Author: raffael
 */

#include <msp430.h>
#include "communication.h"
#include "commands.h"
#include "module_control.h"


volatile unsigned char RXData;
unsigned char TXData;
enum i2c_status_{
	IDLE,  			//not in use, and not to be handled
	NEW_COMMAND, 	//woke up from ISR, therefore treat the request of the mainboard.
} i2c_status;

eps_status_t eps_status;
uint8_t module_status[N_MODULES]; //stores the answers to be sent to an eventual i2c request
uint8_t poke_counter=0;
uint8_t poke_pin_state=0;
uint8_t heater1_counter=0;

void init_timer_A()
{
	TACCR0 += 25000;
	TACCTL0 = CCIE;                     // TACCR0 interrupt enabled
	TACTL = TASSEL_2 + MC_2 + ID_3;     // SMCLK, contmode
}

void init_timer_B()
{
	TBCCR0 += 25000;
	TBCCTL0 = CCIE;                     // TACCR0 interrupt enabled
	TBCTL = TBSSEL_2 + MC_2 + ID_3;     // SMCLK, contmode
}

void init_i2c()
{
	i2c_status = IDLE;
	P3SEL |= 0x06;                            // Assign I2C pins to USCI_B0
	UCB0CTL1 |= UCSWRST;                      // Enable SW reset
	UCB0CTL0 = UCMODE_3 + UCSYNC;             // I2C Slave, synchronous mode
	UCB0I2COA = 0x48;                         // Own Address is 048h
	UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
	UCB0I2CIE |= UCSTTIE;                     // Enable STT interrupt
	IE2 |= UCB0TXIE;                          // Enable TX interrupt
	TXData = 0xff;                            // Used to hold TX data
}


void check_i2c_command() // sets the response.
{
	switch(RXData)
	{
		case ALIVE: poke_counter=COMM_OK;break;
		case M3V3_1_OFF: TXData=COMM_OK; break;
		case M3V3_2_OFF: TXData=COMM_OK; break;
		case M5V_OFF: TXData=COMM_OK; break;
		case M11V_OFF: TXData=COMM_OK; break;
		case HEAT_1_OFF: TXData=COMM_OK; break;
		case HEAT_2_OFF: TXData=COMM_OK; break;
		case HEAT_3_OFF: TXData=COMM_OK; break;
		case M3V3_1_ON: TXData = module_status[M331]; break;
		case M3V3_2_ON: TXData = module_status[M332]; break;
		case M5V_ON: TXData = module_status[M5]; break;
		case M11V_ON: TXData = module_status[M11]; break;
		case HEAT_1_ON: TXData=module_status[HT1]; break;
		case HEAT_2_ON: TXData=module_status[HT2]; break;
		case HEAT_3_ON: TXData=module_status[HT3]; break;

		case V_BAT:TXData = eps_status.v_bat; break;
		case V_SC: TXData = eps_status.v_solar; break;
		case I_IN: TXData = eps_status.current_in; break;
		case I_OUT: TXData = eps_status.current_out; break;
		case AEXT1: TXData = eps_status.analog_ext1; break;
		case AEXT2: TXData = eps_status.analog_ext2; break;
		case T_BAT:TXData = eps_status.t_bat;break;

		#ifndef ANALOG_6
		case AEXT3: TXData = eps_status.analog_ext3; break;
		case AEXT4: TXData = eps_status.analog_ext4; break;
		#endif

		default: TXData=UNKNOWN_COMMAND;break;
	}

}

void execute_i2c_command()
{
	if(i2c_status == NEW_COMMAND)
	{
		switch(RXData)
		{
			case ALIVE: poke_counter=0;break;
			case M3V3_1_OFF: module_control(PORT_3V3_1_EN,PIN_3V3_1_EN,OFF); break;
			case M3V3_2_OFF: module_control(PORT_3V3_2_EN,PIN_3V3_2_EN,OFF); break;
			case M5V_OFF: module_control(PORT_5V_EN,PIN_5V_EN,OFF); break;
			case M11V_OFF: module_control(PORT_11V_EN,PIN_11V_EN,OFF); break;
			case HEAT_1_OFF: module_control(PORT_HEATER_1_EN,PIN_HEATER_1_EN,OFF); break;
			case HEAT_2_OFF: module_control(PORT_HEATER_2_EN,PIN_HEATER_2_EN,OFF); break;
			case HEAT_3_OFF: module_control(PORT_HEATER_3_EN,PIN_HEATER_3_EN,OFF); break;
			case M3V3_1_ON: module_control(PORT_3V3_1_EN,PIN_3V3_1_EN,ON, module_status[M331]); break;
			case M3V3_2_ON: module_control(PORT_3V3_2_EN,PIN_3V3_2_EN,ON, module_status[M332]); break;
			case M5V_ON: module_control(PORT_5V_EN,PIN_5V_EN, ON, module_status[M5]); break;
			case M11V_ON: module_control(PORT_11V_EN,PIN_11V_EN, ON, module_status[M11]); break;
			case HEAT_1_ON: module_control(PORT_HEATER_1_EN,PIN_HEATER_1_EN, ON ,module_status[HT1]); init_timer_B(); heater1_counter=10; break;
			case HEAT_2_ON: module_control(PORT_HEATER_2_EN,PIN_HEATER_2_EN, ON ,module_status[HT2]); break;
			case HEAT_3_ON: module_control(PORT_HEATER_3_EN,PIN_HEATER_3_EN, ON , module_status[HT3]); break;


			default: break;
		}
		i2c_status = IDLE;
	}

	module_control(MASTER_POKE_PORT, MASTER_POKE_PIN,poke_pin_state, COMM_OK);
	if(poke_counter>=4){
		module_control(PORT_3V3_M_EN,PIN_3V3_M_EN, poke_pin_state, COMM_OK);
		poke_counter=0;
	}

}


// USCI_B0 Data ISR
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
	UCB0TXBUF = TXData;                     // TX data
	i2c_status = NEW_COMMAND;
  __bic_SR_register_on_exit(CPUOFF);        // Exit LPM0 and execute request

}

// USCI_B0 State ISR
#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR(void)

{
	UCB0STAT &= ~UCSTTIFG;                    // Clear start condition int flag
	RXData = UCB0RXBUF;						//read command
	check_i2c_command();
}


// Timer B0 interrupt service routine
#pragma vector=TIMERB0_VECTOR
__interrupt void Timer_B ()
{
	//here we poke the main board and after x attempt without reply we reboot it
	heater1_counter--;
	TBCCR0 += 25000;
	if(heater1_counter==0)
	{
		TBCCTL0 &= ~CCIE;				     				   //disable timer B interrupt
		module_control(PORT_HEATER_1_EN,PIN_HEATER_1_EN,OFF);  //disable heater1
	}
	__bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}


// Timer A0 interrupt service routine
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A ()
{
	//here we poke the main board and after x attempt without reply we reboot it
	poke_pin_state = ~(poke_pin_state);
	if(poke_pin_state)
		poke_counter++;
	TACCR0 += 25000;
	__bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}


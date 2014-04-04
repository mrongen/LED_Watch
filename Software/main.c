
#include <msp430.h>
#include "led_watch.h"

volatile unsigned char sec=1;
volatile unsigned char min=1;
volatile unsigned char hour=12;

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

  pinSetup();
  timerSetup();

  _BIS_SR(LPM3_bits + GIE);                 // Enter LPM3 w/ interrupt

  while(1){ //runs when tilt switch triggers (in main to not interrupt the RTC counting)
	  showTime(30000);
	  _BIS_SR(LPM3_bits + GIE);                 // Enter LPM3 w/ interrupt
  }

}



//turns on led corresponding to there time value on the watchface (charliplexed LEDs)
void LEDon(unsigned char led){
	P1REN &=~ ((1<<0) | (1<<1) | (1<<2) | (1<<3)); //pullup/pulldown disable
	P1DIR &=~ ((1<<0)  | (1<<1) | (1<<2) | (1<<3)); //input (Z)
	switch (led){
	case 1:
		P1DIR |= ((1<<3) | (1<<2)); //ouput
		P1OUT &=~ ((1<<3)); //low pin
		P1OUT |= ((1<<2)); //high pin
		break;
	case 2:
		P1DIR |= ((1<<1) | (1<<3)); //ouput
		P1OUT &=~ ((1<<3)); //low pin
		P1OUT |= ((1<<1)); //high pin
		break;
	case 3:
		P1DIR |= ((1<<0) | (1<<1)); //ouput
		P1OUT &=~ ((1<<0)); //low pin
		P1OUT |= ( (1<<1)); //high pin
		break;
	case 4:
		P1DIR |= ((1<<1) | (1<<2) ); //ouput
		P1OUT &=~ ((1<<1)); //low pin
		P1OUT |= ((1<<2)); //high pin
		break;
	case 5:
		P1DIR |= ((1<<2) | (1<<3)); //ouput
		P1OUT &=~ ((1<<2)); //low pin
		P1OUT |= ((1<<3)); //high pin
		break;
	case 6:
		P1DIR |= ((1<<1) | (1<<3)); //ouput
		P1OUT &=~ ((1<<1)); //low pin
		P1OUT |= ((1<<3)); //high pin
		break;
	case 7:
		P1DIR |= ((1<<0) | (1<<2)); //ouput
		P1OUT &=~ ((1<<0)); //low pin
		P1OUT |= ((1<<2) ); //high pin

		break;
	case 8:
		P1DIR |= ((1<<0) | (1<<2) ); //ouput
		P1OUT &=~ ((1<<2)); //low pin
		P1OUT |= ((1<<0)); //high pin
		break;
	case 9:
		P1DIR |= ( (1<<0) | (1<<3)); //ouput
		P1OUT &=~ ((1<<0)); //low pin
		P1OUT |= ( (1<<3)); //high pin
		break;
	case 10:
		P1DIR |= ((1<<0) | (1<<3)); //ouput
		P1OUT &=~ ((1<<3)); //low pin
		P1OUT |= ((1<<0) ); //high pin
		break;
	case 11:
		P1DIR |= ((1<<0) | (1<<1)); //ouput
		P1OUT &=~ ((1<<1)); //low pin
		P1OUT |= ((1<<0)); //high pin
		break;
	case 12:
		P1DIR |= ((1<<1) | (1<<2)); //ouput
		P1OUT &=~ ((1<<2)); //low pin
		P1OUT |= ((1<<1)); //high pin
		break;

	default: //turn off leds
		P1REN |= ((1<<0) | (1<<1) | (1<<2) | (1<<3)); //pullup/pulldown enable
		P1OUT &=~ ((1<<0) | (1<<1) | (1<<2) | (1<<3)); //pulldown pin save power
		break;
	}
}

//setup of in/output pins
void pinSetup(){
	  P1DIR |= ((1<<0) | (1<<1) | (1<<2) | (1<<3));  //output
	  P1REN |= ((1<<0) | (1<<1) | (1<<2) | (1<<3)); //pullup/pulldown enable
	  P1OUT &=~ ((1<<0) | (1<<1) | (1<<2) | (1<<3)); //pulldown pin save power

	  P1REN|= ((1<<4) | (1<<5) | (1<<6) | (1<<7)); //pullup/pulldown enable (inputs and unused pins
	  P1OUT |= ((1<<4) | (1<<5) | (1<<6)| (1<<7)); //pullup
}

//Setup timer interrupt for 1sec
void timerSetup(){
	  CCTL0 = CCIE;                             // CCR0 interrupt enabled
	  CCR0 = 32767;							//1 sec
	  TACTL = TASSEL_1 + MC_1;           // ACLK, upmode

}

unsigned char calcMinLED(){
/*	char temp = min;
	unsigned char ledMin = 0;
	while(temp >= 5){
		temp -= 5;
		ledMin++;
	}

	if(ledMin == 0){
		ledMin = 12;
	}*/

	return min/5;
}

//shows time with LEDs
void showTime(unsigned int delayTime){

	unsigned char ledMin = calcMinLED();

	if(ledMin == 0){
		ledMin = 12;
	}

	unsigned int i = 0;
	unsigned char cnt=0; //PWM variable

	while(i<=delayTime){ //time to show time with LEDs

		if(cnt==15){ //low PWM on minute LED
			cnt=0;
			LEDon(ledMin);
		}else{ //high PWM on hour LED
			LEDon(hour);
		}
		cnt++;
		unsigned int j = 0;
		while(j<100){ //time LEDs are active
			j++;
		}
		LEDon(0); //all LED off
		while(j<100){ //time LEDs are active
			j++;
		}
		i++;
	}
}

// Timer A0 interrupt service routine every second
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
	static unsigned char filter = 0;

	sec++;
	if(sec > 60){
		sec=1;
		min++;
		if(min > 60){
			min=1;
			hour++;
			if(hour > 12){
				hour = 1;
			}
		}
	}

	if((P1IN & (1<<5)) == 0){
		min=1;
	}
	while((P1IN & (1<<5)) == 0){ //set time pad shorted
		min+=5;
		if(min > 60){
			min=1;
			hour++;
			if(hour > 12){
				hour = 1;
			}
		}
		showTime(5000);
	}
	if ((P1IN & (1<<7)) == 0){ //Pull tilt switch if low show time
		filter++;
		if(filter == 2){
			filter=0;
			__bic_SR_register_on_exit(LPM3_bits); //returns to main loop to show time
		}
	}
  //MCU goes back to sleep
}






/*
 * led_watch.h
 *
 *  Created on: 13 okt 2013
 *      Author: Anders
 */

#ifndef LED_WATCH_H_
void LEDon(unsigned char led);
void pinSetup();
void timerSetup();
void showTime();
unsigned char calcMinLED();
#define LED_WATCH_H_



#endif /* LED_WATCH_H_ */

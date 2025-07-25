#ifndef LED_H_
#define LED_H_
#define Right 0
#define Left 1
//--------------------------------------
#include "stm32f1xx.h"
#include "time.h"
//--------------------------------------
void segchar (void);
void ledprint(uint16_t number);
void ledprinttime(Time *time);
void ledprintf(float number, uint8_t alignment);
void ledprintt(uint8_t R0,uint8_t R1, uint8_t R2, uint8_t R3);
//--------------------------------------
#endif
#ifndef RCC_H_
#define RCC_H_
#include "stm32f1xx.h"
#define Delay_MicroSecond(microsecond) Delay_Tact(microsecond*72)

extern uint32_t Millis;

void Clock_Init_To72Mhz(void);
void Clock_Reset(void);

void DWT_Init(void);
void SysTick_Init(void);

void Delay_While(uint32_t cicles);
void Delay_MilliSecond(uint32_t millisecond);
#pragma inline=forced
inline void Delay_Tact(uint32_t tact)
{
  DWT->CYCCNT = 0;
  while(DWT->CYCCNT<tact){}
}
#endif
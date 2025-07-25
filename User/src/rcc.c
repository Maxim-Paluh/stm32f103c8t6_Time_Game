#include "rcc.h"
uint32_t Millis = 0; 

/*************************
Priority:
1. Start the HSE generator
2. Configure PLL
3. Start PLL
4. Configure FLASH
5. Configure prescaler bus
6. Switch to work from PLL
**************************/
void Clock_Init_To72Mhz(void)
{
  //Start the HSE generator
  SET_BIT(RCC->CR, RCC_CR_HSEON);
  while (READ_BIT(RCC->CR, RCC_CR_HSERDY) == RESET) {}
  
  //Configure PLL
  MODIFY_REG(RCC->CFGR, 
             RCC_CFGR_PLLMULL_0 | RCC_CFGR_PLLMULL_1 | RCC_CFGR_PLLMULL_2 | RCC_CFGR_PLLMULL_3,
             0x07<<RCC_CFGR_PLLMULL_Pos | RCC_CFGR_PLLSRC); //PLL Mul = 9 | PLL source
  
  //Start PLL
  SET_BIT(RCC->CR, RCC_CR_PLLON); 
  while(READ_BIT(RCC->CR, RCC_CR_PLLRDY) == RESET) {}
  
  //Configure FLASH
  MODIFY_REG(FLASH->ACR, 
             FLASH_ACR_LATENCY_0| FLASH_ACR_LATENCY_1 | FLASH_ACR_LATENCY_2,
             0x02<<FLASH_ACR_LATENCY_Pos);
  
  //Configure prescaler bus
  MODIFY_REG(RCC->CFGR, 
            RCC_CFGR_PPRE2_0 | RCC_CFGR_PPRE2_1 | RCC_CFGR_PPRE2_2 | // APB high-speed prescaler (APB2)
            RCC_CFGR_PPRE1_0 | RCC_CFGR_PPRE1_1 | RCC_CFGR_PPRE1_2 | // APB low-speed prescaler (APB1)
            RCC_CFGR_HPRE_0  | RCC_CFGR_HPRE_1  | RCC_CFGR_HPRE_2  | RCC_CFGR_HPRE_3 | //AHB prescaler
            RCC_CFGR_ADCPRE_0 | RCC_CFGR_ADCPRE_1, //ADC prescaler
            
            0x00<<RCC_CFGR_PPRE2_Pos | 0x04<<RCC_CFGR_PPRE1_Pos | 0x00<<RCC_CFGR_HPRE_Pos | RCC_CFGR_ADCPRE_1);
  
  //Switch to work from PLL
  MODIFY_REG(RCC->CFGR, RCC_CFGR_SW_0 | RCC_CFGR_SW_1, 0x02<<RCC_CFGR_SW_Pos);
  while((RCC->CFGR & RCC_CFGR_SWS_Msk) != (0x02<<RCC_CFGR_SWS_Pos)){}
  
  SystemCoreClockUpdate();
}

void Clock_Reset(void)
{
  // Start HSI if he disabled
  if(READ_BIT(RCC->CR, RCC_CR_HSIRDY)==RESET)
  {
    RCC->CR |= RCC_CR_HSION;
    while (READ_BIT(RCC->CR, RCC_CR_HSIRDY) == RESET) {}
  }
  
  //Switch to work from HSI
  CLEAR_BIT(RCC->CFGR, RCC_CFGR_SW_0 | RCC_CFGR_SW_1);
  while((RCC->CFGR & RCC_CFGR_SWS_Msk) != RESET){}
  
  //Configure prescaler bus
  CLEAR_BIT(RCC->CFGR, 
            RCC_CFGR_PPRE2_0 | RCC_CFGR_PPRE2_1 | RCC_CFGR_PPRE2_2 |
            RCC_CFGR_PPRE1_0 | RCC_CFGR_PPRE1_1 | RCC_CFGR_PPRE1_2 |
            RCC_CFGR_HPRE_0  | RCC_CFGR_HPRE_1  | RCC_CFGR_HPRE_2  | RCC_CFGR_HPRE_3);
  
  //Configure FLASH
  CLEAR_BIT(FLASH->ACR, FLASH_ACR_LATENCY_0| FLASH_ACR_LATENCY_1 | FLASH_ACR_LATENCY_2);
  
  //Stop PLL
  CLEAR_BIT(RCC->CR, RCC_CR_PLLON); 
  while(READ_BIT(RCC->CR, RCC_CR_PLLRDY) != RESET) {}
  
  //Clrear configure PLL
  CLEAR_BIT(RCC->CFGR, 
             RCC_CFGR_PLLMULL_0 | RCC_CFGR_PLLMULL_1 | RCC_CFGR_PLLMULL_2 | RCC_CFGR_PLLMULL_3 | RCC_CFGR_PLLSRC);
  
    //Stop the HSE generator
  CLEAR_BIT(RCC->CR, RCC_CR_HSEON);
  while (READ_BIT(RCC->CR, RCC_CR_HSERDY) != RESET) {}
  
  SystemCoreClockUpdate();
}

void DWT_Init(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void SysTick_Init(void)
{
  MODIFY_REG(SysTick->LOAD,SysTick_LOAD_RELOAD_Msk, SystemCoreClock/1000 - 1);
  CLEAR_BIT(SysTick->VAL, SysTick_VAL_CURRENT_Msk);
  SET_BIT(SysTick->CTRL, SysTick_CTRL_CLKSOURCE_Msk  //Processor clock (AHB)
                         | SysTick_CTRL_ENABLE_Msk // // enable SysTick
                         | SysTick_CTRL_TICKINT_Msk); // enable Interrupt (IRQ)
}

void Delay_While(__IO uint32_t cicles)
{
  while(cicles)
  {
    cicles--;
  }  
}

void Delay_MilliSecond(uint32_t millisecond)
{
  MODIFY_REG(SysTick->VAL, SysTick_VAL_CURRENT_Msk, SystemCoreClock/1000 - 1);
  uint32_t tempmillis = Millis;
  while((Millis-tempmillis)<millisecond){}
}

void SysTick_Handler(void)
{
  Millis++;
}
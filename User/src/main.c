#include "rcc.h"
#include "led.h"
#include "config.h"

#include "time.h"
#include "Animation.h"
#include "melody.h"

#include <stdbool.h>
#include <stdint.h>


void Init_Debug(void){
  //Clocking AFIO
  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_AFIOEN); // start clocking AFIO
  uint32_t tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_AFIOEN); //Pause 
  // Debug NOJTAG: JTAG-DP Disabled and SW-DP Enabled 
  CLEAR_BIT(AFIO->MAPR,AFIO_MAPR_SWJ_CFG);
  SET_BIT(AFIO->MAPR, AFIO_MAPR_SWJ_CFG_JTAGDISABLE);

}

void Init_GPIO(void)
{
  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPAEN); // Clocking PORT_A
  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPBEN); // Clocking PORT_B
  //---------------------------------------------------------------------------------------------------------------------
  CLEAR_BIT(GPIOA->CRL, GPIO_CRL_MODE3);         // MODE3 = 00 (вхід)
  CLEAR_BIT(GPIOA->CRL, GPIO_CRL_CNF3_0);         // CNF3 = 10 (вхід з підтяжкою)
  SET_BIT(GPIOA->CRL, GPIO_CRL_CNF3_1);         // 10: Input with pull-up / pull-down
  SET_BIT(GPIOA->ODR, GPIO_ODR_ODR3);            // Підтяжка до Vcc через ODR
  // ==== Прив’язка EXTI3 до PA3 ====
  CLEAR_BIT(AFIO->EXTICR[0], AFIO_EXTICR1_EXTI3); // Очистити EXTICR1 для EXTI3
  SET_BIT(AFIO->EXTICR[0], AFIO_EXTICR1_EXTI3_PA);// Прив'язка EXTI3 → PA3
  // ==== Налаштування EXTI ====
  SET_BIT(EXTI->IMR, EXTI_IMR_MR3);              // Дозвіл маски переривання
  SET_BIT(EXTI->FTSR, EXTI_FTSR_TR3);            // Спадний фронт (falling edge)
  CLEAR_BIT(EXTI->RTSR, EXTI_RTSR_TR3);          // Висхідний фронт не потрібен
  // ==== NVIC дозволи ====
  NVIC_EnableIRQ(EXTI3_IRQn);                    // Дозвіл у системному контролері
  NVIC_SetPriority(EXTI3_IRQn, 1);               // Пріоритет, якщо потрібно
  //---------------------------------------------------------------------------------------------------------------------
  // Init PB8-15 MODE 11 CNF 01
  SET_BIT(GPIOB->CRH, 
              GPIO_CRH_MODE15_1 | GPIO_CRH_MODE15_0
            | GPIO_CRH_MODE14_1 | GPIO_CRH_MODE14_0
            | GPIO_CRH_MODE13_1 | GPIO_CRH_MODE13_0
            | GPIO_CRH_MODE12_1 | GPIO_CRH_MODE12_0
            | GPIO_CRH_MODE11_1 | GPIO_CRH_MODE11_0
            | GPIO_CRH_MODE10_1 | GPIO_CRH_MODE10_0
            | GPIO_CRH_MODE9_1 | GPIO_CRH_MODE9_0
            | GPIO_CRH_MODE8_1 | GPIO_CRH_MODE8_0);
  // Init PB8-15  ODR 11111111
  CLEAR_REG(GPIOB->ODR);
  SET_BIT(GPIOB->ODR, GPIO_ODR_ODR8 | GPIO_ODR_ODR9 | GPIO_ODR_ODR10 | GPIO_ODR_ODR11 | GPIO_ODR_ODR12 | GPIO_ODR_ODR13 | GPIO_ODR_ODR14 | GPIO_ODR_ODR15); 
}

#define TIM_EnableIT_UPDATE(TIMx) SET_BIT(TIMx->DIER, TIM_DIER_UIE)
#define TIM_EnableCounter(TIMx) SET_BIT(TIMx->CR1, TIM_CR1_CEN)
#define TIM_DisableCounter(TIMx) CLEAR_BIT(TIMx->CR1, TIM_CR1_CEN)

static void TIM1_Init(void) // PWM 
{
    //Init PA8-11 MODE 11 CNF 10 
  MODIFY_REG(GPIOA->CRH, GPIO_CRH_CNF8_0 | GPIO_CRH_CNF9_0 | GPIO_CRH_CNF10_0 | GPIO_CRH_CNF11_0,
             GPIO_CRH_MODE8_1 | GPIO_CRH_MODE8_0 | GPIO_CRH_CNF8_1 
           | GPIO_CRH_MODE9_1 | GPIO_CRH_MODE9_0 | GPIO_CRH_CNF9_1
           | GPIO_CRH_MODE10_1 | GPIO_CRH_MODE10_0 | GPIO_CRH_CNF10_1
           | GPIO_CRH_MODE11_1 | GPIO_CRH_MODE11_0 | GPIO_CRH_CNF11_1);
  
  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM1EN);
  WRITE_REG(TIM1->PSC, 71);
  WRITE_REG(TIM1->ARR, 1999);
  
  SET_BIT(TIM1->CCMR1,  TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | 
                        TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2  );
  
   SET_BIT(TIM1->CCMR2,  TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 |
                         TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2 );
  
  TIM1->CCR1 = 0;
  TIM1->CCR2 = 0;
  TIM1->CCR3 = 0;
  TIM1->CCR4 = 0;
  
  SET_BIT(TIM1->BDTR, TIM_BDTR_MOE_Msk);
  
  TIM1->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;
  TIM_EnableCounter(TIM1);
}

static void TIM2_Init(void) // Led switch
{ 
  // Clocking Timer2
  SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM2EN);
  // Enable IRQ in NVIC
  NVIC_EnableIRQ(TIM2_IRQn);
  
  WRITE_REG(TIM2->PSC, 3599);
  WRITE_REG(TIM2->ARR, 99);
  
  TIM_EnableIT_UPDATE(TIM2);
  TIM_EnableCounter(TIM2);
}

static void TIM3_Init(void) // TimerGame
{ 
  // Clocking Timer2
  SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM3EN);
  
  WRITE_REG(TIM3->PSC, 7199); // 72 MHz / (7199 + 1) = 10 kHz → 0.1 ms такт
  WRITE_REG(TIM3->ARR, 0xFFFF); // Максимальний лічильник

  TIM_EnableCounter(TIM3);
}

static void TIM4_Init(void) //MelodyWin
{
  MODIFY_REG(GPIOB->CRL,
             GPIO_CRL_CNF6 | GPIO_CRL_MODE6,
             GPIO_CRL_CNF6_1 | GPIO_CRL_MODE6); // CNF6 = 10 (AF), MODE6 = 11 (Output 50 MHz)
  
  // --- Clock TIM4 ---
  SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM4EN);
  
  // --- PWM конфіг ---
  WRITE_REG(TIM4->PSC, 71);                // Prescaler → 1 МГц
  WRITE_REG(TIM4->ARR, 1000 - 1);          // Період → 1 кГц
  WRITE_REG(TIM4->CCR1, 500);              // Заповнення → 50%
  
  MODIFY_REG(TIM4->CCMR1,
             TIM_CCMR1_OC1M | TIM_CCMR1_OC1PE,
             TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1PE); // PWM mode 1
  
   SET_BIT(TIM4->CR1, TIM_CR1_ARPE);        // Автоматичне перезавантаження
  //SET_BIT(TIM4->CCER, TIM_CCER_CC1E);      // Увімкнути вихід на канал CH1
  //TIM_EnableCounter(TIM4);
}

void Init(void){
  Clock_Init_To72Mhz(); //1
  SysTick_Init(); //2
  //DWT_Init(); //3
  Init_Debug(); //4
  
  Config_Init();
  Init_GPIO();
  TIM1_Init();
  TIM2_Init();
  TIM3_Init();
  TIM4_Init();
}
void GetResult(Time* timeGame, Time* timeResult);
uint8_t status = 0;
Time timeGame = {
.Second =0,
.partSecond =0,
.IsWin = false
};
Time timeResult = {
.Second =0,
.partSecond =0,
.IsWin = false
};

// анімація
Animation a1Down =      { anim1, 6, 0,  0, 100,  10, 10, 0, true, 0}; // затухаюча
Animation a1Up =        { anim1, 6, 0,  0, 200, -10, 10, 0, true, 0}; // прискорююча
Animation a2Down=       { anim2, 8, 0,  0, 100,  10, 10, 0, true, 0}; // затухаюча
Animation a2Up =        { anim2, 8, 0,  0, 200, -10, 10, 0, true, 0}; // прискорююча
Animation a3Down =      { anim3, 12, 0, 0, 100,  10, 10, 0, true, 0}; // затухаюча
Animation a3DUp =       { anim3, 12, 0, 0, 200, -10, 10, 0, true, 0}; // прискорююча

volatile uint32_t lastMillis_LimitFPS = 0;              //для обмеження частоти оновлення цифр при грі
volatile uint32_t lastMillis_Blink = 0;                 //для блимання цифр при програші
volatile uint32_t lastMillis_WaitMenu = 0;              //довгі затримки для переходу між режимами при довгій бездіяльності
volatile uint32_t lastMillis_debounce = 0;              // від брязкоду

bool blinkState = true; // змінна котра треба для миготіння якщо гравець програв

int main()
{
  Init();
  while (1)
  {
    switch (status)
    {
    case 0:                             // ОЧІКУВАННЯ початку гри або переходу в налаштування 
      if(a1Up.active)                   // якщо довго нічого не відбувається то перехів д режим сну
        Animate(&a1Up);
      else if(a1Down.active)
        Animate(&a1Down);
      else if (a2Up.active)
        Animate(&a2Up);
      else if (a2Down.active)
        Animate(&a2Down);
      else if (a3DUp.active)
        Animate(&a3DUp);
      else if (a3Down.active)
        Animate(&a3Down);
      else
      {
        a1Up.delay = 200;
        a1Up.active = true;
        a1Down.delay = 100;
        a1Down.active = true;
        
        a2Up.delay = 200;
        a2Up.active = true;
        a2Down.delay = 100;
        a2Down.active = true;
        
        a3DUp.delay = 200;
        a3DUp.active = true;
        a3Down.delay = 100;
        a3Down.active = true;
      }
      if(isPlaying)
        StopMelody();
      
      break;
    case 1:                             // ГРА, бачимо таймер з налічуваними секундами і інкрементуємо його
      timeGame.partSecond = TIM3->CNT;
      if (timeGame.partSecond >= 10000)
      {
        TIM3->CNT = 0;
        timeGame.Second++;
      }
      if(timeGame.Second>=30) // автовиграш при >=10, або стандартно 30 секунд якщо користувач вирішив не грати.
      {
        timeGame.partSecond = TIM3->CNT;
        lastMillis_WaitMenu = Millis;
        GetResult(&timeGame, &timeResult);
        status = 2;
      }
      if(Millis - lastMillis_LimitFPS >30)
      {
        lastMillis_LimitFPS = Millis;
        ledprinttime(&timeGame); // Виводимо секунди на дисплей
      }
      break;
    case 2:                             // РЕЗУЛЬТАТ, займер зупиняється і виводиться час, гравець сам бачить чи виграв чи ні, якщо виграв зіграємо мелодію (реалізукти потім)
      if (timeResult.IsWin)                   // через деякий час переходимо в режим очікування якщо користувач не натисне кнопку для нової гри
      {
        blinkState = true;
        (Millis - lastMillis_WaitMenu >= 18000) ? StopMelody() : PlayMelody();
      }
      else
        if (Millis - lastMillis_Blink >= (blinkState ? 800 : 200))
        {
          lastMillis_Blink = Millis;
          blinkState = !blinkState;
        }
      if (blinkState)
        ledprinttime(&timeResult);  // показати час
      else
        ledprintt(0xFF, 0xFF, 0xFF, 0xFF); // "очистити" індикатор
      
      if (Millis - lastMillis_WaitMenu >= (timeResult.IsWin ? 300000 : 30000)) // якщо виграв то чекаємо 5 хвилин, якщо програв то чекаємо 30 секунд
      {
        lastMillis_WaitMenu = Millis;
        status = 0;
      }
      break;
    case 3:                             // НАЛАШУТВАННЯ яскравості і складності гри (сюди можна потрапити лише з режиму очікування або результату і тут своє меню )
      break;
    }
  } 
}

void GetResult(Time* timeGame, Time* timeResult)
{
  timeResult->IsWin = false;
  timeResult->Second = timeGame->Second;
  timeResult->partSecond = timeGame->partSecond;
  
  if(timeResult->Second!=3)
  {
    return;
  }
  else
  {
     uint16_t mask = (1 << (config.Difficulty)) - 1;
     uint16_t temp_partSecond = timeResult->partSecond & ~mask;
     if(temp_partSecond == 0)
     {
       timeResult->partSecond = 0;
       timeResult->IsWin = true;
       return;
     }
     else {
        timeResult->IsWin = false;
        return;
     }   
  }
}

void EXTI3_IRQHandler(void)
{
  if (EXTI->PR & EXTI_PR_PR3)
  {
    SET_BIT(EXTI->PR, EXTI_PR_PR3);
    
    uint32_t now = Millis;
    if((now - lastMillis_debounce > 300))
    {
      lastMillis_debounce = now;
      
      switch (status)
      {
      case 0: timeGame.Second=0; timeGame.partSecond=0; TIM3->CNT = 0;                  status = 1;    break;
      case 1: lastMillis_WaitMenu = Millis; GetResult(&timeGame, &timeResult);          status = 2;    break;
      case 2: lastMillis_WaitMenu = Millis;                                             status = 0;    break;
      case 3:                                                                           break;  //ігноруємо в цей режим потрапляємо іншою кнопкою і інших режимів                                                                
      }
      
    }
  }
}

void TIM2_IRQHandler(void)
{
  if(READ_BIT(TIM2->SR, TIM_SR_UIF))
  {
    CLEAR_BIT(TIM2->SR, TIM_SR_UIF);
    segchar();
  }
} 
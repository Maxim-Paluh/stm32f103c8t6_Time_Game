#include "led.h"
#include "math.h"
#include "config.h"
uint8_t digit = 0;
uint8_t R[4] = {255,255,255,255};
//==============================

volatile uint32_t* CCR_primary[4] = { &TIM1->CCR1, &TIM1->CCR2,  &TIM1->CCR3, &TIM1->CCR4};
volatile uint32_t* CCR_secondary[4] = { &TIM1->CCR4, &TIM1->CCR1, &TIM1->CCR2, &TIM1->CCR3};

void segchar (void)
{
  SET_BIT(GPIOB->ODR, 0xFF00);

  *CCR_secondary[digit] = 0;
  *CCR_primary[digit]   = config.Brightness;
  
  MODIFY_REG(GPIOB->ODR, 0xFF00, R[digit]<<8);
  
  
  digit++;
  if(digit>3)
    digit =0;
}

//==============================
uint8_t CharToHex(uint8_t Char)
{
  switch(Char)
  {
  case 0:
    return 0xC0;
  case 1:
    return 0xF9;
  case 2:
    return 0xA4;
  case 3:
    return 0xB0;
  case 4:
    return 0x99;
  case 5:
    return 0x92;
  case 6:
    return 0x82;
  case 7:
    return 0xF8;
  case 8:
    return 0x80;
  case 9:
    return 0x90;
    
  case 10:
    return 0x40; 
  case 11:
    return 0x79;
  case 12:
    return 0x24;
  case 13:
    return 0x30;
  case 14:
    return 0x19;
  case 15:
    return 0x12;
  case 16:
    return 0x2;
  case 17:
    return 0x78;
  case 18:
    return 0x0;
  case 19:
    return 0x10;
  default:
    return 255;
  }
}

void ledprint(uint16_t number)
{
  R[3] = CharToHex(number%10);
  //-----------------------------------
  if(number<10)
    R[2] = 255;
  else
  R[2] = CharToHex(number%100/10);
  //-----------------------------------
  if(number<100)
    R[1] = 255;
  else
  R[1] = CharToHex(number%1000/100);
  //-----------------------------------
  if(number<1000)
    R[0] = 255;
  else
  R[0] = CharToHex(number%10000/1000);
}

void ledprinttime(Time *time)
{
  uint8_t sec = time->Second%100/10;
    if(sec==0)
      if(config.ShowNull)
        R[0]= 0xC0;
      else
        R[0]= 0xFF;
    else
      R[0] =  CharToHex(sec);
    R[1] =  CharToHex((time->Second%10)+10);
    R[2] = CharToHex(time->partSecond%10000/1000);
    R[3] = CharToHex(time->partSecond%1000/100);
}

void ledprintf(float number, uint8_t alignment)
{
  float left,right;
  right=modff (number,&left);
  
  if(left>99)
    ledprint((uint16_t)number);
  else if(left>9)
  {
    R[2] = CharToHex((uint8_t)(right*10));
    R[1] = CharToHex(((uint8_t)left%10)+10);
    R[0] = CharToHex((uint8_t)left%100/10);
  }
  else
  {
    if(alignment==0)
    {
      R[2] = CharToHex((uint8_t)(right*10));
      R[1] = CharToHex(((uint8_t)left)+10);
      R[0] = 255;
    }
    else
    {
      R[2] = CharToHex((uint8_t)(right*100)%10);
      R[1] = CharToHex((uint8_t)(right*10));
      R[0] = CharToHex(((uint8_t)left)+10);
    }
  }
}

void ledprintt(uint8_t R0,uint8_t R1, uint8_t R2, uint8_t R3)
{
  R[0] = R0;
  R[1] = R1;
  R[2] = R2;
  R[3] = R3;
}
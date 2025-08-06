#include "stm32f1xx.h"
#include "config.h"

/* Brightness
0-1999 де 0 min Brightness 1999 max Brightness
*/
/* Difficulty
╔════════════╤═════════════════════╤════════════════╤════════════════╤════════════════╗
║ Difficulty │     Bitmask         │ Window (ticks) │ Error (ticks)  │ Error (ms)     ║
╟────────────┼─────────────────────┼────────────────┼────────────────┼────────────────╢
║     0      │ 0b00000000          │       1        │       1        │     0.1 ms     ║
║     1      │ 0b00000001          │       2        │       2        │     0.2 ms     ║
║     2      │ 0b00000011          │       4        │       4        │     0.4 ms     ║
║     3      │ 0b00000111          │       8        │       8        │     0.8 ms     ║
║     4      │ 0b00001111          │      16        │      16        │     1.6 ms     ║
║     5      │ 0b00011111          │      32        │      32        │     3.2 ms     ║
║     6      │ 0b00111111          │      64        │      64        │     6.4 ms     ║
║     7      │ 0b01111111          │     128        │     128        │    12.8 ms     ║
╚════════════╧═════════════════════╧════════════════╧════════════════╧════════════════╝
*/

Config config = {
    .Brightness =     0,
    .Difficulty =     0,
    .AnimationChar =  0,
    .AnimationPoint = 0,
    .ShowNull = 0
};

uint8_t AnimationConvertOut(Config* animationConfig)
{
  if(animationConfig->AnimationChar == 0 && animationConfig->AnimationPoint == 1) // Цифри 00:00 або 0:00 з моргаючою крапкою
    return 0;
  if(animationConfig->AnimationChar == 0 && animationConfig->AnimationPoint == 2) // Цифри 00:00 або 0:00 з постійно увімкнутою крапкою
    return 1;
  if(animationConfig->AnimationChar == 1 && animationConfig->AnimationPoint == 0) // анімація завантаження з вимкнутою крапкою // по замовчуванню
    return 2;
   if(animationConfig->AnimationChar == 1 && animationConfig->AnimationPoint == 1) // анімація завантаження з моргаючою крапкою
    return 3;
   if(animationConfig->AnimationChar == 1 && animationConfig->AnimationPoint == 2) // анімація завантаження з постійно увімкнутою крапкою
    return 4;
  return 2;
}

void AnimationConvertIn(uint8_t animation, Config* animationConfig)
{
  switch (animation)
  {
  case 0:
    animationConfig->AnimationChar = 0; animationConfig->AnimationPoint = 1;
    break;
  case 1:
    animationConfig->AnimationChar = 0; animationConfig->AnimationPoint = 2;
    break;
  case 2:
    animationConfig->AnimationChar = 1; animationConfig->AnimationPoint = 0;
    break;
  case 3:
    animationConfig->AnimationChar = 1; animationConfig->AnimationPoint = 1;
    break;
  case 4:
    animationConfig->AnimationChar = 1; animationConfig->AnimationPoint = 2;
    break;
  default:
    animationConfig->AnimationChar = 1; animationConfig->AnimationPoint = 0;
    break;  
  }
}


//------------------------------------------------------------------------------
static void Flash_Unlock(void) {
    if (FLASH->CR & FLASH_CR_LOCK) {
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
    }
}

static void Flash_Lock(void) {
    FLASH->CR |= FLASH_CR_LOCK;
}

static void Flash_ErasePage(uint32_t address) {
    SET_BIT(FLASH->CR, FLASH_CR_PER);
    FLASH->AR = address;
    SET_BIT(FLASH->CR, FLASH_CR_STRT);
    while (FLASH->SR & FLASH_SR_BSY);
    CLEAR_BIT(FLASH->CR, FLASH_CR_PER);
}

static void Flash_WriteWord(uint32_t address, uint32_t data) {
    SET_BIT(FLASH->CR, FLASH_CR_PG);
    *(volatile uint16_t*)address = (uint16_t)(data & 0xFFFF);
    while (FLASH->SR & FLASH_SR_BSY);
    *(volatile uint16_t*)(address + 2) = (uint16_t)(data >> 16);
    while (FLASH->SR & FLASH_SR_BSY);
    CLEAR_BIT(FLASH->CR, FLASH_CR_PG);
}

void Flash_WriteConfig(const Config* cfg) {
    __disable_irq();         // 🔒 Вимкнути всі глобальні переривання
    Flash_Unlock();
    Flash_ErasePage(FLASH_CONFIG_ADDR);

    Flash_WriteWord(FLASH_CONFIG_ADDR, (cfg->AnimationChar));
    Flash_WriteWord(FLASH_CONFIG_ADDR + 4, (cfg->AnimationPoint));
    Flash_WriteWord(FLASH_CONFIG_ADDR + 8, (cfg->Brightness));
    Flash_WriteWord(FLASH_CONFIG_ADDR + 12, (cfg->Difficulty));
    Flash_WriteWord(FLASH_CONFIG_ADDR + 16, (cfg->ShowNull));
    Flash_WriteWord(FLASH_CONFIG_ADDR + 20, (cfg->SleepAfter));
    Flash_WriteWord(FLASH_CONFIG_ADDR + 24, CONFIG_SIGNATURE);

    Flash_Lock();
    __enable_irq();          // 🔓 Увімкнути всі глобальні переривання
}

static void Flash_ReadConfig(Config* cfg) {
    cfg->AnimationChar    =     *(volatile uint32_t*)(FLASH_CONFIG_ADDR);
    cfg->AnimationPoint  =      *(volatile uint32_t*)(FLASH_CONFIG_ADDR + 4);
    cfg->Brightness  =          *(volatile uint32_t*)(FLASH_CONFIG_ADDR + 8);
    cfg->Difficulty =           *(volatile uint32_t*)(FLASH_CONFIG_ADDR + 12);
    cfg->ShowNull =             *(volatile uint32_t*)(FLASH_CONFIG_ADDR + 16);
    cfg->SleepAfter =           *(volatile uint32_t*)(FLASH_CONFIG_ADDR + 20);
    cfg->Signature     =        *(volatile uint32_t*)(FLASH_CONFIG_ADDR + 24);
}

void Config_Init(void)
{
    Flash_ReadConfig(&config);

    if (config.Signature != CONFIG_SIGNATURE)
    {
        config.AnimationChar = 1;
        config.AnimationPoint = 0;
        config.Brightness   = 1999;
        config.Difficulty = 63;
        config.ShowNull = 0;
        config.SleepAfter = 5;
        config.Signature    = CONFIG_SIGNATURE;

        Flash_WriteConfig(&config);
    }
}
//------------------------------------------------------------------------------


   
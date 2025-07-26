#include "stm32f1xx.h"
#include "config.h"

/*
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
║     8      │ 0b11111111          │     256        │     256        │    25.6 ms     ║
║     9      │ 0b0001_1111_1111    │     512        │     512        │    51.2 ms     ║
║    10      │ 0b0011_1111_1111    │    1024        │    1024        │   102.4 ms     ║
║    11      │ 0b0111_1111_1111    │    2048        │    2048        │   204.8 ms     ║
║    12      │ 0b1111_1111_1111    │    4096        │    4096        │   409.6 ms     ║
╚════════════╧═════════════════════╧════════════════╧════════════════╧════════════════╝

Notes:
- 1 tick = 0.1 ms (10,000 ticks per second)
- mask = (1 << Difficulty) - 1
- window = mask + 1 → range where (partSecond & mask) == 0
*/


Config config = {
    .Brightness=0,
    .Difficulty=0
};

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

static void Flash_WriteConfig(const Config* cfg) {
    Flash_Unlock();
    Flash_ErasePage(FLASH_CONFIG_ADDR);

    Flash_WriteWord(FLASH_CONFIG_ADDR, (cfg->Brightness));
    Flash_WriteWord(FLASH_CONFIG_ADDR + 4, (cfg->Difficulty));
    Flash_WriteWord(FLASH_CONFIG_ADDR + 8, CONFIG_SIGNATURE);

    Flash_Lock();
}

static void Flash_ReadConfig(Config* cfg) {
    cfg->Brightness    = *(volatile uint32_t*)(FLASH_CONFIG_ADDR);
    cfg->Difficulty  = *(volatile uint32_t*)(FLASH_CONFIG_ADDR + 4);
    cfg->signature     = *(volatile uint32_t*)(FLASH_CONFIG_ADDR + 8);
}

void Config_Init(void)
{
    Flash_ReadConfig(&config);

    if (config.signature != CONFIG_SIGNATURE)
    {
        config.Brightness   = 1999;
        config.Difficulty = 7;
        config.signature    = CONFIG_SIGNATURE;

        Flash_WriteConfig(&config);
    }
}



   
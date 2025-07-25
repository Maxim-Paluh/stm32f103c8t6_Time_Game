#include "stm32f1xx.h"
#include "config.h"


Config config = {
    .brightness=0,
    .timer_period=0
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

    Flash_WriteWord(FLASH_CONFIG_ADDR, (cfg->brightness));
    Flash_WriteWord(FLASH_CONFIG_ADDR + 4, (cfg->timer_period));
    Flash_WriteWord(FLASH_CONFIG_ADDR + 8, CONFIG_SIGNATURE);

    Flash_Lock();
}

static void Flash_ReadConfig(Config* cfg) {
    cfg->brightness    = *(volatile uint32_t*)(FLASH_CONFIG_ADDR);
    cfg->timer_period  = *(volatile uint32_t*)(FLASH_CONFIG_ADDR + 4);
    cfg->signature     = *(volatile uint32_t*)(FLASH_CONFIG_ADDR + 8);
}

void Config_Init(void)
{
    Flash_ReadConfig(&config);

    if (config.signature != CONFIG_SIGNATURE)
    {
        config.brightness   = 1999;
        config.timer_period = 500;
        config.signature    = CONFIG_SIGNATURE;

        Flash_WriteConfig(&config);
    }
}


   
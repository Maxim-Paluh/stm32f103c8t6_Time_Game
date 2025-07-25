#ifndef CONFIG_H_
#define CONFIG_H_
#include <stdint.h>

#define CONFIG_SIGNATURE 0xA5A5A5A6
#define FLASH_CONFIG_ADDR  ((uint32_t)0x0801FC00) // Остання сторінка (сторінка 63)

typedef struct {
    uint16_t brightness;    
    uint16_t timer_period;
    uint32_t signature; // маркер автентичності наприклад 0xA5A5A5A5
} Config;

extern Config config;

void Config_Init(void);

#endif 

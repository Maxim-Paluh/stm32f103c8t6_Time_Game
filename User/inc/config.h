#ifndef CONFIG_H_
#define CONFIG_H_
#include <stdint.h>

#define CONFIG_SIGNATURE 0xA5A5A5A5
#define FLASH_CONFIG_ADDR  ((uint32_t)0x0801FC00) // Остання сторінка (сторінка 63)

typedef struct {
    uint16_t Brightness;    
    uint16_t Difficulty;
    uint32_t signature; // маркер автентичності наприклад 0xA5A5A5A5
    uint16_t AnimationChar;
    uint16_t AnimationPoint;
} Config;

extern Config config;

void Config_Init(void);

#endif 

#ifndef CONFIG_H_
#define CONFIG_H_
#include <stdint.h>

#define CONFIG_SIGNATURE 0xA5A5A5A5
#define FLASH_CONFIG_ADDR  ((uint32_t)0x0801FC00) // Остання сторінка (сторінка 63)

typedef struct {
    uint16_t AnimationChar;     // тип анімації символів
    uint16_t AnimationPoint;    // тип анімації крапки
    uint16_t Brightness;        // яскравість
    uint16_t Difficulty;        // складність
    uint16_t ShowNull;          // показувати 0 на початку поки секунд менше 10
    uint16_t SleepAfter;        // заснути через вказану кількість хвилин
    uint32_t Signature;         // маркер автентичності наприклад 0xA5A5A5A5
} Config;

extern Config config;

void Config_Init(void);
void Flash_WriteConfig(const Config* cfg);
uint8_t AnimationConvertOut(Config* animationConfig);
void AnimationConvertIn(uint8_t animation, Config* animationConfig);

#endif 

#ifndef ANIMATION_H_
#define ANIMATION_H_
#include <stdint.h>
#include <stdbool.h>

extern const uint8_t anim1[][4];
extern const uint8_t anim2[][4];
extern const uint8_t anim3[][4];

typedef struct {
    const uint8_t (*frames)[4];         // кадри анімації
    uint8_t frameCount;                 // кількість кадрів
    uint8_t currentIndex;               // індекс поточного кадру
    uint32_t lastMillis;                // змінна для затримок
    uint16_t delay;                     // базова затримка, початкова
    int16_t speedStep;                  // для прискорення/сповільнення додається до delay
    uint8_t countRepet;                 // кількість повторів
    uint16_t currentRepet;              // поточна кількість повторів 
    bool active;                        // чи активна (при завершенні всіх повторів стає false)
    bool endPausa;                      // пауза в кінці, щоб не зникав останній кадр в останньому повторі
} Animation;

void Animate(Animation*);

#endif 

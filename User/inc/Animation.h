#ifndef ANIMATION_H_
#define ANIMATION_H_
#include <stdint.h>
#include <stdbool.h>

extern const uint8_t anim1[][4];
extern const uint8_t anim2[][4];
extern const uint8_t anim3[][4];

typedef struct {
    const uint8_t (*frames)[4];  // кадри анімації
    uint8_t     frameCount;      // кількість кадрів
    uint8_t     frameCurrent;    // індекс поточного кадру
    uint32_t    lastMillis;      // змінна для затримок
    uint16_t    delayBase;       // базова затримка, початкова
    uint8_t     repetCount;      // кількість повторів
    uint16_t    repetCurrent;    // поточна кількість повторів 
    uint16_t    delayCurrent;    // поточна затримка
    int16_t     delayStep;       // для прискорення/сповільнення керує delayCurrent в момент збільшення repetCurrent
    bool        active;          // чи активна (при завершенні всіх повторів стає false)
    bool        endPausa;        // пауза в кінці, щоб не зникав останній кадр в останньому повторі
} Animation;

void Animate(Animation*);

#endif 

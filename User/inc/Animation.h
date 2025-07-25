#ifndef ANIMATION_H_
#define ANIMATION_H_
#include <stdint.h>
#include <stdbool.h>

extern const uint8_t anim1[][4];
extern const uint8_t anim2[][4];
extern const uint8_t anim3[][4];

typedef struct {
    const uint8_t (*frames)[4]; // кадри анімації
    uint8_t frameCount;
    uint8_t currentIndex;
    uint32_t lastMillis;
    uint16_t delay;
    int16_t speedStep; // для прискорення/сповільнення
    uint8_t countRepet;
    uint16_t currentRepet;
    bool active;
    uint8_t endPausa;
} Animation;

void Animate(Animation*);

#endif 

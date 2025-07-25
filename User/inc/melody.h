#ifndef MELODY_H_
#define MELODY_H_
#include <stdint.h>
#include <stdbool.h>

extern bool isPlaying;


typedef struct {
    uint16_t freq;
    uint16_t duration; // в мс
} Note;

void PlayMelody(void);
void StopMelody(void);
#endif

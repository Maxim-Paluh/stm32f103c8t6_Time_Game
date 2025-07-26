#ifndef TIME_H_
#define TIME_H_
#include <stdbool.h>
typedef struct {
    uint8_t Second;    
    uint16_t partSecond;
    bool IsWin;
} Time;
#endif
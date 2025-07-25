#include "melody.h"
#include "stm32f1xx.h"
#include "rcc.h"
bool isPlaying = false;
static uint8_t currentNote = 0;
static uint32_t noteStartMillis = 0;

static Note melody[] = {
  {660, 150}, {660, 150}, {0, 100}, {660, 150}, // E E - E
  {0, 100}, {520, 150}, {660, 150}, {0, 100},   // - C E -
  {770, 150}, {0, 100}, {770, 150}, {800, 150}, // G - G G#
  {840, 200}, {880, 250}, {0, 100},             // A A#
  {1046, 300},                                   // C6 — кульмінація
  {988, 150}, {880, 150}, {784, 150},            // B A G
  {660, 200}, {0, 150},                          // E - пауза
  {660, 150}, {780, 150}, {660, 150},            // E G E
  {880, 200}, {0, 100},                          // A пауза
  {1046, 300}, {0, 300}                          // C6 — фінал
};

static void PlayNote(uint16_t freq)
{
  if (freq == 0)
  {
    CLEAR_BIT(TIM4->CR1, TIM_CR1_CEN);     // зупинити таймер
    CLEAR_BIT(TIM4->CCER, TIM_CCER_CC1E);  // вимкнути PWM вихід
    return;
  }
  
  uint16_t arr = 1000000 / freq - 1;
  WRITE_REG(TIM4->ARR, arr);
  WRITE_REG(TIM4->CCR1, arr / 2);            // 50% заповнення
  
  SET_BIT(TIM4->CCER, TIM_CCER_CC1E);        // увімкнути PWM-вихід
  SET_BIT(TIM4->CR1, TIM_CR1_CEN);           // запустити таймер
}

void PlayMelody(void)
{
  isPlaying = true;
  if (Millis - noteStartMillis >= melody[currentNote].duration)
  {
    currentNote++;
    noteStartMillis = Millis;
    if (currentNote >= sizeof(melody) / sizeof(melody[0]))
    {
      currentNote = 0; // або мелодія завершена
      PlayNote(0);     // тиша
    }
    else
    {
      PlayNote(melody[currentNote].freq);
    }
  }
}

void StopMelody(void)
{
  isPlaying = false;
  PlayNote(0);
  currentNote = 0;
  noteStartMillis = 0;
}
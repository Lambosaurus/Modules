#ifndef BUZZER_H
#define BUZZER_H

#include "STM32X.h"

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

typedef struct {
	uint32_t freq;
	uint32_t duration;
}Note_t;

/*
 * PUBLIC FUNCTIONS
 */

void Buzzer_Init(void);
void Buzzer_Deinit(void);
void Buzzer_Play(const Note_t * notes, uint16_t count);
void Buzzer_Beep(uint32_t freq, uint32_t duration);
void Buzzer_Halt(void);
bool Buzzer_IsBusy(void);
void Buzzer_Update(void);

#endif //BUZZER_H

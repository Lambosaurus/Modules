#include "Buzzer.h"
#include "Core.h"
#include "TIM.h"

/*
 * PRIVATE DEFINITIONS
 */

#define DUTY_MAX		10

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

void Buzzer_StartNote(const Note_t * note);

/*
 * PRIVATE VARIABLES
 */

static struct {
	uint16_t count;
	const Note_t * notes;
	struct {
		uint32_t period;
		uint32_t last;
	} timer;
} gState;

/*
 * PUBLIC FUNCTIONS
 */

void Buzzer_Init(void)
{
	gState.count = 0;
}

void Buzzer_Deinit(void)
{
	if (Buzzer_IsBusy())
	{
		Buzzer_Halt();
	}
}

void Buzzer_Play(const Note_t * notes, uint16_t count)
{
	if (Buzzer_IsBusy())
	{
		Buzzer_Halt();
	}

	gState.count = count;
	gState.notes = notes;

	TIM_Init(BUZZER_TIM, 1000, DUTY_MAX - 1);
	TIM_EnablePwm(BUZZER_TIM, BUZZER_TIM_CH, BUZZER_PIN, BUZZER_PIN_AF);

	Buzzer_StartNote(gState.notes);
}

void Buzzer_Beep(uint32_t freq, uint32_t duration)
{
	if (Buzzer_IsBusy())
	{
		Buzzer_Halt();
	}
	// Using a note on the stack is safe - so long as its a single note.
	Note_t note = {
		.freq = freq,
		.duration = duration,
	};
	Buzzer_Play(&note, 1);
}

void Buzzer_Halt(void)
{
	TIM_Stop(BUZZER_TIM);
	TIM_Deinit(BUZZER_TIM);
	GPIO_Deinit(BUZZER_PIN);

	gState.count = 0;
}

bool Buzzer_IsBusy(void)
{
	return gState.count > 0;
}

void Buzzer_Update(void)
{
	if (gState.count > 0)
	{
		if ((CORE_GetTick() - gState.timer.last) > gState.timer.period)
		{
			gState.count -= 1;
			if (gState.count > 0)
			{
				TIM_Stop(BUZZER_TIM);
				Buzzer_StartNote(++gState.notes);
			}
			else
			{
				Buzzer_Halt();
			}
		}
	}
}

/*
 * PRIVATE FUNCTIONS
 */

void Buzzer_StartNote(const Note_t * note)
{
	gState.timer.period = note->duration;
	gState.timer.last = CORE_GetTick();
	if (note->freq == 0)
	{
		TIM_SetPulse(BUZZER_TIM, BUZZER_TIM_CH, 0);
	}
	else
	{
		TIM_SetPulse(BUZZER_TIM, BUZZER_TIM_CH, DUTY_MAX / 2);
		TIM_SetFreq(BUZZER_TIM, note->freq * DUTY_MAX);
	}
	TIM_Start(BUZZER_TIM);
}


/*
 * INTERRUPT ROUTINES
 */

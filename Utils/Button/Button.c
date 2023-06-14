
#include "Button.h"
#include "Core.h"
#include "GPIO.h"

/*
 * PRIVATE DEFINITIONS
 */

#ifndef BTN_DEBOUNCE_TIME
#define BTN_DEBOUNCE_TIME 20
#endif

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static inline bool Button_IsHeld(Button_t * btn);

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

void Button_Init(Button_t * btn, GPIO_Pin_t pin)
{
	Button_InitAdv(btn, pin, GPIO_Pull_Up, false);
}

void Button_InitAdv(Button_t * btn, GPIO_Pin_t pin, GPIO_Pull_t pull, GPIO_State_t heldState)
{
	GPIO_EnableInput(pin, pull);

	btn->pin = pin;
	btn->heldState = heldState;
	btn->state = Button_IsHeld(btn) ? Button_State_Held : Button_State_None;
	btn->changeTime = CORE_GetTick();
}

void Button_Deinit(Button_t * btn)
{
	GPIO_Deinit(btn->pin);
}

Button_State_t Button_Update(Button_t * btn)
{
	bool isDown = Button_IsHeld(btn);

	uint32_t now = CORE_GetTick();
	Button_State_t state = btn->state & Button_State_Held;
	bool wasDown = state != Button_State_None;

	if (isDown == wasDown)
	{
		// No state change, bump the timer
		btn->changeTime = now;
	}
	else if (now - btn->changeTime > BTN_DEBOUNCE_TIME)
	{
		btn->changeTime = now;
		state = isDown ? Button_State_Pressed : Button_State_Released;
	}

	btn->state = state;
	return state;
}


/*
 * PRIVATE FUNCTIONS
 */

static inline bool Button_IsHeld(Button_t * btn)
{
	return GPIO_Read(btn->pin) == btn->heldState;
}

/*
 * INTERRUPT ROUTINES
 */

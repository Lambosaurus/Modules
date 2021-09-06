#ifndef BUTTON_H
#define BUTTON_H

#include "STM32X.h"

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

typedef enum {
	Button_State_None     = 0,
	Button_State_Held     = 0x01,
	Button_State_Changed  = 0x02,
	Button_State_Pressed  = Button_State_Changed | Button_State_Held,
	Button_State_Released = Button_State_Changed,
} Button_State_t;

typedef struct {
	GPIO_t * gpio;
	uint32_t pin;
	uint32_t changeTime;
	ButtonState_t state;
	GPIO_State_t heldState;
} Button_t;

/*
 * PUBLIC FUNCTIONS
 */

void Button_Init(Button_t * btn, GPIO_t * gpio, uint32_t pin);
void Button_InitAdv(Button_t * btn, GPIO_t * gpio, uint32_t pin, GPIO_Pull_t pull, GPIO_State_t heldState);
void Button_Deinit(Button_t * btn);
Button_State_t Button_Update(Button_t * btn);

#endif //BUTTON_H

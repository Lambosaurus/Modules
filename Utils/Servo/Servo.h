#ifndef SERVO_H
#define SERVO_H

#include "STM32X.h"
#include "GPIO.h"

/*
 * PUBLIC DEFINITIONS
 */

#ifndef SERVO_PERIOD_MS
#define SERVO_PERIOD_MS 20
#endif

/*
 * PUBLIC TYPES
 */

typedef struct Servo_s Servo_t;

struct Servo_s {
	GPIO_Pin_t pin;
	uint32_t period;
	Servo_t * next;
};

/*
 * PUBLIC FUNCTIONS
 */

void Servo_Init(Servo_t * servo, GPIO_Pin_t pin, uint32_t period_us);
void Servo_Write(Servo_t * servo, uint32_t period_us);
void Servo_Deinit(Servo_t * servo);

#endif //SERVO_H

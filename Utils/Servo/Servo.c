#include "Servo.h"
#include "TIM.h"
#include "GPIO.h"

/*
 * PRIVATE DEFINITIONS
 */

#if (SERVO_PERIOD_MS * 1000 > 0xFFFF)
#error "SERVO_PERIOD_MS is too high for the 16 bit timers"
#endif

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void Servo_StartTimer(void);
static void Servo_StopTimer(void);
static void Servo_Next(void);
static void Servo_Reload(void);
static inline void Servo_StartPulse(void);

/*
 * PRIVATE VARIABLES
 */

static struct {
	Servo_t * head;
	Servo_t * current;
} gServo;

/*
 * PUBLIC FUNCTIONS
 */

void Servo_Init(Servo_t * servo, GPIO_Pin_t pin, uint32_t period_us)
{
	servo->period = period_us;
	servo->pin = pin;
	servo->next = NULL;

	GPIO_EnableOutput(pin, GPIO_PIN_RESET);

	Servo_t ** head = &gServo.head;
	while (*head != NULL) { head = &(*head)->next; }
	*head = servo;

	// If we just became head, then the timer is unstarted.
	if (gServo.head == servo)
	{
		Servo_StartTimer();
	}
}

void Servo_Write(Servo_t * servo, uint32_t period_us)
{
	servo->period = period_us;
}

void Servo_Deinit(Servo_t * servo)
{
	Servo_t ** head = &gServo.head;
	while (*head != servo)
	{
		if (*head == NULL)
		{
			// The servo mustnt have been in the list.
			return;
		}

		head = &(*head)->next;
	}

	CRITICAL_SECTION_BEGIN();

	// Remove ourselves
	*head = servo->next;

	if (gServo.head == NULL)
	{
		// Timer no longer needed.
		Servo_StopTimer();
	}
	else if (gServo.current == servo)
	{
		// Uh oh, this output is active.
		// Move to the next one. Note that we expect Servo_Next to handle a null servo.
		Servo_Next();
	}

	CRITICAL_SECTION_END();

	GPIO_Deinit(servo->pin);
}

/*
 * PRIVATE FUNCTIONS
 */

static void Servo_StartTimer(void)
{
	TIM_Init(SERVO_TIM, 1000000, SERVO_PERIOD_MS * 1000);
	TIM_OnPulse(SERVO_TIM, TIM_CH1, Servo_Next);
	TIM_OnReload(SERVO_TIM, Servo_Reload);

	Servo_Reload();

	TIM_Start(SERVO_TIM);
}

static void Servo_StopTimer(void)
{
	TIM_Stop(SERVO_TIM);
	TIM_Deinit(SERVO_TIM);
	gServo.current = NULL;
}

static inline void Servo_StartPulse(void)
{
	GPIO_Set(gServo.current->pin);
	uint32_t now = TIM_Read(SERVO_TIM);
	TIM_SetPulse(SERVO_TIM, TIM_CH1, now + gServo.current->period);
}

/*
 * INTERRUPT ROUTINES
 */

static void Servo_Next(void)
{
	if (gServo.current == NULL)
	{
		// This has somehow occurred with no next servo.
		// Abandon ship!
		return;
	}

	// Reset the current device.
	GPIO_Reset(gServo.current->pin);

	gServo.current = gServo.current->next;
	if (gServo.current)
	{
		Servo_StartPulse();
	}
}

static void Servo_Reload(void)
{
	// Timer has ended. Restart the chain.
	gServo.current = gServo.head;
	Servo_StartPulse();
}

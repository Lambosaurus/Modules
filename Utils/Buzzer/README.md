# Buzzer
A module to autmomate playing tunes from a piezo buzzer.

## Usage

Notes are given to the buzzer in frequency period pairs.
Note that a frequency of `0` can be used to indicate a rest.

```C
const Note_t notes[] = { {440, 200}, {0, 100}, {440, 200}, {660, 500} };
Buzzer_Init();
Buzzer_Play(&notes, LENGTH(notes));

while (1)
{
    Buzzer_Update();
    ...
    CORE_Idle();
}
```

## Board

The module is dependant on  definitions within `Board.h`

The following template can be used.

```C
// Configure buzzer module
#define BUZZER_GPIO			GPIOA
#define BUZZER_PIN			GPIO_PIN_6
#define BUZZER_TIM			TIM_22
#define BUZZER_TIM_CH		TIM_CH1
#define BUZZER_PIN_AF		GPIO_AF5_TIM22
#define TIM22_ENABLE
```
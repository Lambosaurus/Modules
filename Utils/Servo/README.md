# Servo

This module provides a servo pwm output.

Servos can be added and removed dynamically at runtime without disruption to the sequence.

## Usage

The following demonstrates starting a servo. Any number of servos can be added.

```C
Servo_t servo1;
Servo_t servo2;
Servo_Init(&servo1, PA0, 1500); // Pulse widths set in us.
Servo_Init(&servo2, PA1, 1500);
...
// Update the output value.
Servo_Write(&servo2, 2000);
```

> Warning: Currently there will be issues if the sum of the pulse widths exceed `SERVO_PERIOD_MS`.

# Board

The module is dependent on  definitions within `Board.h`

The following template can be used.

```C
// Timer config
#define TIM14_ENABLE
#define TIM_USE_IRQS

// Servo
#define SERVO_TIM           TIM_14
```
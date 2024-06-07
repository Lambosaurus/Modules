# Random
This implements simple random and helpers, which is independant from the standard libraries.

## Usage

```C
Random_Seed();

uint32_t r = Random_RandInt(0, 100);
```

## Seeding

The random seed is selected in `Random_Seed`. By default, this will use the MCU's UID as a seed.

By defining `RANDOM_TEMP_SEED`, thermal noise from the temperature sensor can be used as a seen instead.
Note that the ADC is expected to be deinitialized during the `Random_Seed` call.


## Board

The module is dependant on  definitions within `Board.h`.

The following template can be used.
Commented out lines are optional.

```C
//#define RANDOM_TEMP_SEED
```
# Random
This implements simple random and helpers, which is independant from the standard libraries.

It supports multiple strategies for seeding, and multiple algorithims.

## Usage

```C
// Note, Random_Seed signature changes if RANDOM_SEED_MANUAL is selected.
Random_Seed();

uint32_t r1 = Random_Read();
uint32_t r2 = Random_RandInt(0, 100);
```

## Seeding

The random seed can be selected from the following definitions. The Manual strategy is the default.

| Strategy | #define    | Comment |
| -------- | ---------- | ------- |
| Manual   | `RANDOM_SEED_MANUAL` | Seed is passed into the seed function by the user. |
| UID      | `RANDOM_SEED_UID`    | Seed is derived from the MCU Unique ID. |
| Temperature | `RANDOM_SEED_TEMP`   | Seed is derived from LSB noise on the temperature channel. Note that the ADC is assumed to be deinitialized. |

## Algorithim

The random algorithim can be selected from the following definitions. The LCG strategy is the default.

| Strategy | #define    | Comment |
| -------- | ---------- | ------- |
| LCG   | `RANDOM_ALG_LCG` | A Linear Congruential Generator. This is small and fast. The same implementation used by glibc. |
| TinyMT32      | `RANDOM_ALG_TMT`    | TinyMT32 implementation will be used. This is a little slower, but performs very well. See [TinyMT github](https://github.com/MersenneTwister-Lab/TinyMT/tree/master) for more info. |

## Board

The module is dependant on  definitions within `Board.h`.

The following template can be used.
One seed strategy and one algorithim should be selected.

```C
// Seed strategy selection
//#define RANDOM_SEED_MANUAL
//#define RANDOM_SEED_UID
//#define RANDOM_SEED_TEMP

// Algorithim selection
//#define RANDOM_ALG_LCG
//#define RANDOM_ALG_TMT
```
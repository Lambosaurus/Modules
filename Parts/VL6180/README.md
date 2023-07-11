# VL6180
Support for the VL6180 laser ranging module by ST Microelectronics
This uses an I2C interface.

Calibration and crosstalk compensation is not supported.

## Usage

The following config demonstrates a polled sampling.

```C
I2C_Init(VL6180_I2C, I2C_Mode_Fast);

VL6180_Init();

while (1)
{
    VL6180_Start()

    while (!VL6180_IsReady())
    {
        CORE_Idle();
    }

    uint32_t range;
    VL6180_Read(&range);
    ...

    CORE_Delay(100);
}
```

## Board

The module is dependant on  definitions within `Board.h`

The following template can be used.

```C
// VL6180 interface
#define VL6180_I2C          I2C_1

// Configure I2C1
#define I2C1_PINS		(PB6 | PB7)
#define I2C1_AF			GPIO_AF1_I2C1
```

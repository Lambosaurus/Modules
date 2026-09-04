# Bitbang UART

Bitbangs UART on some GPIO. Helpful in a pinch.

## Usage

This is intended to be a replacement for the UART module when required.

This uses a timer and interrupts, and performance at higher bitrates is not guaranteed.


```C
BUART_Init(9600);

const uint8_t tx[] = { 0x01, 0x02, 0x03 };
BUART_Write(tx, sizeof(tx));

uint8_t rx[3];
uint32_t read = BUART_Read(rx, sizeof(rx))
```

The UART configuration is fixed at 8 data bits, no parity, and 1 stop bit (8N1).

## Board

The module is dependant on definitions within `Board.h`

The following template can be used.

```C
// GPIO config
#define GPIO_USE_IRQS
#define GPIO_IRQ1_ENABLE	// IRQ required for the RX pin

// TIM config
#define TIM_USE_IRQS
#define TIM1_ENABLE

// Configure bit banged UART
#define BUART_TX_PIN	PA0
#define BUART_RX_PIN	PA1
#define BUART_TIM		TIM_1
// #define BUART_RX_BFR_SIZE     64
```

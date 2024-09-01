# Console

This module provides a consistant console API.

It can be configured for USB or UART output.

## Initialization

For a UART console, you'll need to select the UART and baud using `CONSOLE_UART` and `CONSOLE_BAUD` definitions. The UART will be initialized within `Console_Init`.

For USB CDC console, simply define `CONSOLE_USB`. The `USB_Init` call is expected to be done by the user. `Console_Init` will not assume this.

The print and scan calls are safe to call even if the module is uninitialized, making this safe to use in other modules that are not aware of the console state.

## Line printing

The following code demonstrates the print functions.

```C
Console_Init();

// Print a fixed string
Console_Prints("hello");
// Print a formatted string.
Console_Printf("%d", 100);
```

Lines will be ended with `\r\n`.

The formatted output of `Console_Printf` must not exceed `CONSOLE_TX_BFR`

## Line scanning

`CONSOLE_RX_BFR` defines the maximum line length, and must be defined to enable line parsing.

`Console_Scans` does a non-blocking parse of the input stream, and returns a null terminated string if a valid line is found.

A valid line is terminated either a `\r` or `\n`, and has a non-zero length.

```C
Console_Init();

while (1)
{
    // This returns the line once parsed.
    const char * line = Console_Scans();
    if (line)
    {
        Console_Printf("read: %s", line);
    }
}

...

while (1)
{
    // This does argument parsing, and returns the number of parsed arguments.
    int number;
    if (Console_Scanf("%d", &number) == 1)
    {
        Console_Printf("read: %d", number);
    }
}

```

## Raw calls

Read and write calls are available via `Console_Read` and `Console_Write`.

These are intended to be used by other internal modules.

These do not handle line endings, and are unsafe without checking `Console_IsEnabled`.

# Board

The module is dependant on  definitions within `Board.h`

The following template can be used.

```C
// UART config
#define UART1_PINS	 		    (PA9 | PA10)
#define UART1_AF		 	    GPIO_AF4_USART1

// Console config
#define CONSOLE_UART          UART_1
#define CONSOLE_BAUD          115200
//#define CONSOLE_USB
//#define CONSOLE_RX_BFR        128
//#define CONSOLE_TX_BFR        128
```
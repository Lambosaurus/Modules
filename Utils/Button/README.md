# Button
This is a simple button module. It includes debouncing, and state change flags.

## Usage
The default targets a button connected to ground, though an advanced init function supports other requirements.

```C

Button_t b;
Button_Init(&b, PA0);

while (1)
{
    if (Button_Update(&b) == Button_State_Pressed)
    {
        ...
    }

    ...
    CORE_Idle();
}

```

Note that the debouncing is handled in `Button_Update`, so this should be called relatively frequently.
Note that `Button_State_t` is bitmask of flags that indicate the button state and whether a transition has occurred.

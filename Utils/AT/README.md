# AT
This provides a framework for dealing with AT command modems.

It aims to allow you to produce code that is safe and easy to reason about, while still being performant and non-blocking.

# Usage

This is expected to be run within a state machine, where each state is called repeatedly until the state is advanced.
All functions are designed to be called repeatedly while their state is true.

The snippet below shows a common use case.

```c

typedef enum {
    User_State_Powerup,
    User_State_WaitForReady,
    User_State_AT,
    User_State_SetRegister,
    User_State_GetRegister,
    User_State_Error,
    User_State_Standby,
} User_State_t;

// Runs a single iteration of the supplied state.
// Returns the next state to be run.
User_State_t User_RunState(User_State_t state)
{
    switch (state)
    {
    case User_State_Powerup:
        // Wait one second in this step.
        AT_SetTimeout(1000);
        if (AT_GetTimeout())
        {
            return ++state;
        }
        return state;

    case User_State_WaitForReady:
        // <- READY              
        switch (AT_ExpectMatch("READY"))          
        {
            case AT_Unexpected:
            case AT_Pending:    return state;
            default:            return User_State_Error;
            case AT_Ok:         return ++state;
        }

    case User_State_AT:
        // -> AT
        // <- OK
        AT_Command("");                 
        switch (AT_ExpectOk())          
        {
            case AT_Pending:    return state;
            default:            return User_State_Error;
            case AT_Ok:         return ++state;
        }

    case User_State_SetRegister:
        // -> AT+R0=10
        // <- OK
        AT_Commandf("+R%d=%d", 0, 10);
        switch (AT_ExpectOk())
        {
            case AT_Pending:    return state;
            default:            return User_State_Error;
            case AT_Ok:         return ++state;
        }

    case User_State_GetRegister:
        // -> AT+R0?
        // <- +R0=10
        // <- OK
        AT_Commandf("+R%d?", 0);
        int reg, value;
        switch (AT_ExpectResponsef(2, "+R%d=%d", &reg, &value))
        {
            case AT_Pending:    return state;
            default:            return User_State_Error;
            case AT_Ok:
                printf("Register %d set to %d\r\n", reg, value);
                return ++state;
        }

    case User_State_Standby:
    case User_State_Error:
        return state;
    }
}

void User_Run(void)
{
    AT_Init();

    User_State_t state = User_State_AT;
    while (state != User_State_Standby)
    {
        // The code within this loop is non-blocking
        // Other non-blocking code can be run in parallel here.

        User_State_t new_state = User_RunState(state);
        if (new_state != state)
        {
            // When entering into a new command state, the command needs to be restarted.
            state = new_state;
            AT_StartCommand();
        }
    }

    AT_Deinit();
}
```

# Sending commands

The Command functions send their payload once, and are reset by `AT_StartCommand`.

| Command          | Description                         | Example command | Example output |
| ---------------- | ----------------------------------- | --------------- | -------------- |
| `AT_Command`     | Sends an unformatted command.       | `AT_Command("E0")` | `ATE0\r\n` |
| `AT_Commandf`    | Sends a formatted command.          | `AT_Commandf("+R%d?", 10)` | `AT+R10?\r\n` |
| `AT_CommandRaw`  | Sends unformatted bytes.            | `AT_CommandRaw((uint8_t*)"DATA", 4)` | `DATA` |

Take care that the resulting string must not exceed `AT_TX_BFR` bytes for formatted commands.

# Expecting replies

The Expect functions parse input, and return an `AT_Status_t` to indicate the current parse status.

| Status           | Description                         | Typical user response |
| ---------------- | ----------------------------------- | ---------------- |
| `AT_Pending`     | No event to handle.                 | Continue to run the current state. |
| `AT_Ok`          | Expected content has been found.    | Process any parsed data, and proceed to next state. |
| `AT_Error`       | An explicit `ERROR` string is found.| Abort this state.  |
| `AT_Unexpected`  | The parsed content did not match the supplied format. | Continue if bad matches may be ignored, otherwise abort. |
| `AT_Overflow`    | The recieve bytes exceeded `AT_RX_BFR` in a single parse. | Abort this state. |
| `AT_Timeout`     | This state has timed out.           | Abort this state. |

All recieve functions tolerate `\n` or `\r\n`, and discard empty lines.

| Expect               | Description                                  | Example Expect             | Example Input | Example Output |
| -------------------- | -------------------------------------------- | -------------------------- | ------------- | -------------- |
| `AT_ExpectOk`        | Expects a single `OK`.                       | `AT_ExpectOk()`            | `OK\r\n`      |                | 
| `AT_ExpectResponse`  | Expects an arbitrary line, followed by `OK`. | `AT_ExpectResponse(&line)` | `CONTENT\r\nOK\r\n` | line: `CONTENT` | 
| `AT_ExpectResponsef` | Expects an formatted line, followed by `OK`. At least `min_args` must be parsed. | `AT_ExpectResponsef(2, "+R%d=%d", &reg, &value)` | `+R0=10\r\nOK\r\n` | reg: 0, value: 10 |
| `AT_ExpectMatch`     | Expects the supplied sequence                | `AT_ExpectMatch("CONNECT")`| `CONNECT\r\n` |                | 
| `AT_ExpectMatchf`    | Expects a formatted line. At least `min_args` must be parsed.             | `AT_ExpectMatchf(1, "+EVENT=%d", &event)`| `+EVENT=2\r\n` | event: 2 | 
| `AT_ExpectRaw`       | Recieves the next `size` bytes.              | `AT_ExpectRaw(&bfr, 4)`    | `DATA\r\n` | bfr: `DATA` | 

# Timeouts

All Expect commands have a timeout, and this starts at `AT_StartCommand`. This defaults to `AT_TIMEOUT_DEFAULT`, but can be explicitly set with `AT_SetTimeout` when a higher timeout is expected.

This can also be accessed via `AT_GetTimeout` which returns `true` if the timeout has occurred, which allows the timeout to be used non Expect functionality.

# Board

The module is dependant on  definitions within `Board.h`

The following template can be used.
Commented out definitions are optional.

```C
// UART config
#define UART1_PINS	 		    (PA9 | PA10)
#define UART1_AF		 	    GPIO_AF4_USART1

// AT config
#define AT_BAUD				    115200
#define AT_UART				    UART_1
//#define AT_CMD_PREFIX		    "AT"
//#define AT_CMD_SUFFIX		    "\r\n"
//#define AT_TX_BFR			    128
//#define AT_RX_BFR			    128
//#define AT_TIMEOUT_DEFAULT	300
```
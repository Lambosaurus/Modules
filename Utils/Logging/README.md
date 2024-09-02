# Logging

This module provides a consistent logging API.

It's highly configurable via #defines, and unneeded logs and definitions are not compiled.

## Usage

The logging statements are printf like, and support argument formatting.

The following code demonstrates each of the logging statements:

```C
int i = 0;
Log_Info("Information %d", i++);
Log_Warn("Warning %d", i++);
Log_Error("Error %d", i++);
```
The formatted output must not exceed `LOG_TX_BFR`

## Configurations

There are multiple options that can be optionally enabled.

### **Print Timestamp**
Defining `LOG_PRINT_TIMESTAMP` prepends a timestamp to each line.

```txt
[001.000] Information 0
[001.000] Warning 1
[001.001] Error 2
```

### **Print Level**
Defining `LOG_PRINT_LEVEL` prepends the log level to each line.

```txt
 INFO: Information 0
 WARN: Warning 1
ERROR: Error 2
```

### **Print File**
Defining `LOG_PRINT_FILE` prepends the source file to each line.

```txt
'main.c': Information 0
'main.c': Warning 1
'main.c': Error 2
```

### **Print Source**
Defining `LOG_PRINT_SOURCE` prepends a source string to each line.
This string is intended to be a friendly module name to clarify the source of the message.

`LOG_SOURCE` should be defined in each file that includes logging statement. This should **not** be defined in the `Board.h` file.

In the example below, `#define LOG_SOURCE "Tests"` is used.

```txt
Tests: Information 0
Tests: Warning 1
Tests: Error 2
```

### **Combination**
The above options can be combined as required:

All:
```txt
[001.000]  INFO 'main.c' Tests: Information 0
[001.000]  WARN 'main.c' Tests: Warning 1
[001.001] ERROR 'main.c' Tests: Error 2
```

None
```txt
Information 0
Warning 1
Error 2
```

## Log level

The `LOG_MIN_LEVEL` can be used to select which log levels are included.

For example, given `#define LOG_MIN_LEVEL LOG_LEVEL_WARN`, only warning and error messages will be evaluated.

```txt
[001.000]  WARN: Warning 0
[001.000] ERROR: Error 1
```

Note that any expressions within the log arguments will no longer be evaluated.


## Dependencies

This is dependent on the [Console](../Console/) module.

# Board

The module is dependent on  definitions within `Board.h`

The following template can be used.

```C
// Logging
#define LOG_MIN_LEVEL           LOG_LEVEL_INFO
//#define LOG_PRINT_TIMESTAMP
//#define LOG_PRINT_LEVEL
//#define LOG_PRINT_FILE
//#define LOG_PRINT_SOURCE
//#define LOG_TX_BFR              128
```
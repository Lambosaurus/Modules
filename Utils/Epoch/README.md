# Epoch
Converts a RTC datetime into a seconds epoch.

## Usage

Note that the epoch is not a unix epoch. Its zero year is defined by `RTC_YEAR_MIN`. This will typically be `2000`.

The following example shows usage of this module.

```C
DateTime_t dt;
RTC_Read(&dt);

// Convert the RTC DR into seconds.
uint32_t seconds = Epoch_FromDateTime(&dt);

seconds += 250;

// Convert the epoch back into a RTC DT.
Epoch_ToDateTime(&dt, seconds);
```

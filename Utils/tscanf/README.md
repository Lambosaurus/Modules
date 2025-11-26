# tscanf
This is a sscanf-like function, but aims to be much smaller.

## Usage

tscanf should behave like sscanf, but with far fewer formats implemented.

It does not do variable whitespace matching like sscanf does, and does literal character matching when not parsing formatted units.

```C
while(1)
{
    const char * src = "20250102";

    int year, month, day;
    if (tscanf(src, "%04d%02d%02d", &year, &month, &day) == 3)
    {
        // year = 2025
        // month = 1
        // day = 2
    }
}
```

## Formats

The width specifier will always be parsed. If not required or implemented for the given format, it will be ignored.

| Format     | Argument | Character | Width | Description |
| ---------- | -------- | --------- | ----- | ----------- |
| Decimal    | int*     | `d`       | Yes   | Parses a decimal integer, with optional sign. |
| Position   | int*     | `n`       | No    | Stores the number of characters read so far. |
| Percent    | -        | `%`       | No    | Parses a literal `%`. |

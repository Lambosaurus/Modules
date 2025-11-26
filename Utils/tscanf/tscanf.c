#include "tscanf.h"

#include <stdarg.h>
#include <stdbool.h>

/*
 * PRIVATE DEFINITIONS
 */

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

int parse_decimal(const char ** str, int width, int * value);

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

int tscanf(const char * str, const char * fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	int matched = 0;
	const char * start = str;

	while (*fmt)
	{
		char fch = *fmt++;
		if (fch == '%')
		{
			int width;
			if (!parse_decimal(&fmt, -1, &width))
			    width = -1;

			switch (*fmt++)
			{
			case 'd': // Decimal integer
			{
			    int value;
				if (!parse_decimal(&str, width, &value))
					goto end;

				matched++;
				*va_arg(va, int*) = value;
				break;
			}
			case 'n': // Number of consumed characters (does not increment match)
			{
				*va_arg(va, int*) = str - start;
				break;
			}
			case '%': // Match a literal % (does not increment match)
			{
				if (*str++ != '%')
					goto end;
				break;
			}
			default: // Unsupported format
				goto end;
			}

		}
		else // Literal character to match
		{
			// Just match characters
			if (*str++ != fch)
				goto end;
		}
	}

end:
	va_end(va);
	return matched;
}

/*
 * PRIVATE FUNCTIONS
 */

int parse_decimal(const char ** str, int width, int * value)
{
    const char * head = *str;
	const char * start = head;

	bool negative = false;

    if (*head == '+' || *head == '-') {
        negative = (*head == '-');
        head++;
        width -= 1;
    }

	int v = 0;
	while (width < 0 || width--)
	{
		if (*head < '0' || *head > '9')
		{
			if (width < 0)
				break; // End reached.
			else
				return 0;
		}
		v = (v * 10) + (*head++ - '0');
	}

	*value = negative ? -v : v;
	*str = head;
	return head - start;
}


/*
 * INTERRUPT ROUTINES
 */


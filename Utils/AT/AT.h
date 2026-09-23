#ifndef AT_H
#define AT_H

#include "STM32X.h"

/*
 * PUBLIC DEFINITIONS
 */

typedef bool(*AT_URCCallback_t)(const char * line);

/*
 * PUBLIC TYPES
 */

typedef enum {
	AT_Pending, 		// No result or match found yet
	AT_Ok, 				// A valid response is found.
	AT_Error, 			// An explicit "ERROR" sequence has been found.
	AT_Unexpected,		// An non-matching sequence has been found
	AT_Overflow,		// A read exceeded the rx buffer size
	AT_Timeout,			// The given timeout has been exceeded.
} AT_Status_t;

/*
 * PUBLIC FUNCTIONS
 */

void AT_Init(void);
void AT_Deinit(void);

void AT_SetUrcHandler(AT_URCCallback_t callback);

void AT_StartCommand(void);
void AT_SetTimeout(uint32_t timeout);
uint32_t AT_GetTimeout(void);
bool AT_TimeoutElapsed(void);

void AT_Command(const char * cmd);
void AT_Commandf(const char * cmd, ...)
	_ATTRIBUTE ((__format__ (__printf__, 1, 2)));
void AT_CommandRaw(const uint8_t * content, uint32_t size);

AT_Status_t AT_ExpectOk(void);
AT_Status_t AT_ExpectResponse(const char * expected);
AT_Status_t AT_ExpectResponsel(char ** response);
AT_Status_t AT_ExpectResponsef(uint32_t min_args, const char * fmt, ...)
	_ATTRIBUTE ((__format__ (__scanf__, 2, 3)));
AT_Status_t AT_ExpectMatch(const char * expected);
AT_Status_t AT_ExpectMatchf(uint32_t min_args, const char * fmt, ...)
	_ATTRIBUTE ((__format__ (__scanf__, 2, 3)));
AT_Status_t AT_ExpectRaw(uint8_t * bfr, uint32_t size);

/*
 * EXTERN DECLARATIONS
 */

#endif // AT_H






#ifndef CBOR_H
#define CBOR_H

#include <stdint.h>
#include <stdbool.h>

/*
 * PUBLIC DEFINITIONS
 */

#define CBOR_INDEFINITE			(~0)

/*
 * PUBLIC TYPES
 */

typedef struct {
	uint8_t * bfr;
	uint8_t * head;
	uint8_t * end;
} CBOR_t;

/*
 * PUBLIC FUNCTIONS
 */

void CBOR_Start(CBOR_t * cbor, uint8_t * bfr, uint32_t size);
uint32_t CBOR_Finish(CBOR_t * cbor); // Returns the consumed buffer, or 0 on error.

void CBOR_Array(CBOR_t * cbor, uint32_t count);
void CBOR_Map(CBOR_t * cbor, uint32_t count);
void CBOR_Break(CBOR_t * cbor); // Use to terminate indefinitely arrays or maps

void CBOR_String(CBOR_t * cbor, const char * value);
void CBOR_Bytes(CBOR_t * cbor, const uint8_t * value, uint32_t size);
void CBOR_Int(CBOR_t * cbor, int32_t value);
void CBOR_Long(CBOR_t * cbor, int64_t value);
void CBOR_Float(CBOR_t * cbor, float value);
void CBOR_Double(CBOR_t * cbor, double value);
void CBOR_Bool(CBOR_t * cbor, bool value);
void CBOR_Null(CBOR_t * cbor);

/*
 * EXTERN DECLARATIONS
 */


#endif // CBOR_H

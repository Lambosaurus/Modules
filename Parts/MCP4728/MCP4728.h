#ifndef MCP4728_H
#define MCP4728_H

#include "STM32X.h"

#ifdef MCP4728_SUPPORT_ADDRESSING
#include "GPIO.h"
#endif


/*
 * PUBLIC DEFINITIONS
 */

#define MCP4728_BITS		12
#define MCP4728_RES			((1 << MCP4728_BITS) - 1)

/*
 * PUBLIC TYPES
 */

typedef enum {
	MCP4728_Ref_2048mV	= 0x80,
	MCP4728_Ref_4096mV  = 0x90,
	MCP4728_Ref_VDD		= 0x00,
} MCP4728_Ref_t;

typedef enum {
	MCP4728_PD_1K 	= 0x01,
	MCP4728_PD_100K = 0x02,
	MCP4728_PD_500K = 0x03,
} MCP4728_PD_t;

/*
 * PUBLIC FUNCTIONS
 */

bool MCP4728_Init(void);
bool MCP4728_Deinit(void);

bool MCP4728_Write(uint8_t channel, MCP4728_Ref_t ref, uint16_t value);
bool MCP4728_Powerdown(uint8_t channel, MCP4728_PD_t pd);

#ifdef MCP4728_SUPPORT_ADDRESSING
void MCP4728_Select(uint8_t addr);
bool MCP4728_SetAddress(uint8_t new_addr, GPIO_Pin_t ldac_pin);
#endif

/*
 * EXTERN DECLARATIONS
 */

#endif //MCP4728_H

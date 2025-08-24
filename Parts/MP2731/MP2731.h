#ifndef MP2731_H
#define MP2731_H

#include "STM32X.h"

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

typedef struct {
	uint32_t in_mv;
	uint32_t in_ma;
	uint32_t batt_mv;
	uint32_t charge_ma;
	uint32_t sys_mv;
	uint32_t ntc;
} MP2731_Analog_t;

typedef enum {
	MP2731_Status_None 			= 0,

	MP2731_Status_Trickle 		= (1 << 0),
	MP2731_Status_Charging 		= (1 << 1),
	MP2731_Status_ChargeDone 	= (1 << 2),

	MP2731_Status_InputFault 	= (1 << 3),
	MP2731_Status_BatteryFault 	= (1 << 4),
	MP2731_Status_NTCFloat 		= (1 << 5),
	MP2731_Status_ThermalFault 	= (1 << 6),
	MP2731_Status_WatchdogFault = (1 << 7),
	MP2731_Status_OTGFault 		= (1 << 8),
} MP2731_Status_t;

/*
 * PUBLIC FUNCTIONS
 */

bool MP2731_Init(void);
void MP2731_Deinit(void);

MP2731_Status_t MP2731_GetStatus(void);
bool MP2731_ReadAnalog(MP2731_Analog_t * values);

void MP2731_SetInputLimit(uint32_t min_mv, uint32_t max_ma);
void MP2731_SetChargeLimit(uint32_t max_mv, uint32_t precharge_ma, uint32_t max_ma, uint32_t termination_ma);

/*
 * EXTERN DECLARATIONS
 */

#endif // MP2731_H

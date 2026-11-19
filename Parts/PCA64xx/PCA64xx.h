#ifndef PCA64xx_H
#define PCA64xx_H

#include "STM32X.h"

/*
 * PUBLIC DEFINITIONS
 */

#ifndef PCA64XX_BITS
#define PCA64XX_BITS		8
#endif

#define PCA64XX_BYTES		((PCA64XX_BITS + 7) / 8)

/*
 * PUBLIC TYPES
 */

typedef enum {
	PCA64xx_Pin_0	= (1 << 0),
	PCA64xx_Pin_1	= (1 << 1),
	PCA64xx_Pin_2	= (1 << 2),
	PCA64xx_Pin_3	= (1 << 3),
	PCA64xx_Pin_4	= (1 << 4),
	PCA64xx_Pin_5	= (1 << 5),
	PCA64xx_Pin_6	= (1 << 6),
	PCA64xx_Pin_7	= (1 << 7),
#if (PCA64XX_BITS > 8)
	PCA64xx_Pin_8	= (1 << 8),
	PCA64xx_Pin_9	= (1 << 9),
	PCA64xx_Pin_10	= (1 << 10),
	PCA64xx_Pin_11	= (1 << 11),
	PCA64xx_Pin_12	= (1 << 12),
	PCA64xx_Pin_13	= (1 << 13),
	PCA64xx_Pin_14	= (1 << 14),
	PCA64xx_Pin_15	= (1 << 15),
#endif
} PCA64xx_Pin_t;

/*
 * PUBLIC FUNCTIONS
 */

bool PCA64xx_Init(void);
bool PCA64xx_Read(PCA64xx_Pin_t pin);
PCA64xx_Pin_t PCA64xx_ReadPort(PCA64xx_Pin_t pins);
bool PCA64xx_Write(PCA64xx_Pin_t pins, bool state);
bool PCA64xx_Configure(PCA64xx_Pin_t pins, bool output);

/*
 * EXTERN DECLARATIONS
 */

#endif //PCA64xx_H

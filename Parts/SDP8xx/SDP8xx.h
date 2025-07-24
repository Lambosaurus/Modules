#ifndef SDP8XX_H
#define MODULE_H

#include "STM32X.h"

/*
 * PUBLIC DEFINITIONS
 */

typedef enum {
    SDP8XX_Mode_Cont            = 0x361E,
    SDP8XX_Mode_Cont_ATR        = 0x3615,
    SDP8XX_Mode_Cont_MF         = 0x3608,
    SDP8XX_Mode_Cont_MF_ATR     = 0x3603,

    //SDP8XX_Mode_Trig            = 0x362F,
    //SDP8XX_Mode_Trig_CS         = 0x372D,
    //SDP8XX_Mode_Trig_MF         = 0x3624,
    //SDP8XX_Mode_Trig_MF_CS      = 0x3726,
} SDP8XX_Mode_t;

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

bool SDP8XX_Init(SDP8XX_Mode_t mode);
void SDP8XX_Deinit(void);
bool SDP8XX_Read(int32_t * pressure); // in millipascals

/*
 * EXTERN DECLARATIONS
 */

#endif //MODULE_H

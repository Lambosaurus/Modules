#ifndef BG95_H
#define BG95_H

#include "STM32X.h"

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

typedef void (*BG95_RequestCallback_t)(uint32_t code, uint32_t rx_size);

typedef enum {
	BG95_Model_None,
	BG95_Model_M1,
	BG95_Model_M2,
} BG95_Model_t;

/*
 * PUBLIC FUNCTIONS
 */

void BG95_Init(void);
void BG95_Deinit(void);

void BG95_Update(void);
bool BG95_IsBusy(void);
bool BG95_IsConnected(void);

void BG95_Reset(void);
void BG95_Wakeup(void);

BG95_Model_t BG95_GetModel(void);
const char * BG95_GetIMSI(void);
const char * BG95_GetIMEI(void);
const uint8_t * BG95_GetIP(void);

// Starts a HTTP Request.
// url is expected to remain valid until the callback has been called.
// cb will be called once the request has returned, with the response code and the total received buffer length
// Returns true if the request was successfully enqueued. False if the modem was not ready for it.
bool BG95_HttpGet(const char * url, uint8_t * bfr, uint32_t bfr_size, BG95_RequestCallback_t cb);
bool BG95_HttpPut(const char * url, uint8_t * bfr, uint32_t bfr_size, BG95_RequestCallback_t cb);
bool BG95_HttpPost(const char * url, uint8_t * tx, uint32_t tx_size, uint8_t * rx, uint32_t rx_size, BG95_RequestCallback_t cb);
bool BG95_HttpPending(void);

// Configuration options
// Strings must remain valid so long as the BG95 is initialized.
void BG95_SetAuthentication(const char * user, const char * password);

/*
 * EXTERN DECLARATIONS
 */

#endif // MODEM_H






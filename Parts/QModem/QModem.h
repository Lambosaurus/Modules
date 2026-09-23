#ifndef QMODEM_H
#define QMODEM_H

#include "STM32X.h"

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

#if defined(QMOD_MODULE_BG95)
#elif defined(QMOD_MODULE_EG800Q)
#else
#error "Please specify supported modem"
#endif

typedef void (*QMOD_RequestCallback_t)(uint32_t code, uint32_t rx_size);

typedef enum {
	QMOD_Info_Model,
	QMOD_Info_IP,
#ifdef QMOD_GET_IMEI
	QMOD_Info_IMEI,
#endif
#ifdef QMOD_GET_IMSI
	QMOD_Info_IMSI,
#endif
#ifdef QMOD_GET_ICCID
	QMOD_Info_ICCID,
#endif
	QMOD_INFO_COUNT,
} QMOD_Info_t;

/*
 * PUBLIC FUNCTIONS
 */

void QMOD_Init(void);
void QMOD_Deinit(void);

void QMOD_Update(void);
bool QMOD_IsBusy(void);
bool QMOD_IsConnected(void);

void QMOD_Reset(void);
void QMOD_Wakeup(void);

const char * QMOD_GetInfo(QMOD_Info_t info);

#ifdef QMOD_ENABLE_HTTP
// Starts a HTTP Request.
// url is expected to remain valid until the callback has been called.
// cb will be called once the request has returned, with the response code and the total received buffer length
// Returns true if the request was successfully enqueued. False if the modem was not ready for it.
bool QMOD_HttpGet(const char * url, uint8_t * bfr, uint32_t bfr_size, QMOD_RequestCallback_t cb);
bool QMOD_HttpPut(const char * url, uint8_t * bfr, uint32_t bfr_size, QMOD_RequestCallback_t cb);
bool QMOD_HttpPost(const char * url, uint8_t * tx, uint32_t tx_size, uint8_t * rx, uint32_t rx_size, QMOD_RequestCallback_t cb);
bool QMOD_HttpPending(void);

// Configuration options
// Strings must remain valid so long as the QMOD is initialized.
void QMOD_SetAuthentication(const char * user, const char * password);
#endif //QMOD_ENABLE_HTTP

/*
 * EXTERN DECLARATIONS
 */

#endif // QMODEM_H






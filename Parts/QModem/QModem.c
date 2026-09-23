#include "QModem.h"

#include "Core.h"
#include "GPIO.h"
#include "AT.h"
#include "Logging.h"

#include <stdio.h>
#include <stdarg.h>

/*
 * PRIVATE DEFINITIONS
 */

#define LOG_SOURCE 				"QMOD"
#define QMOD_HTTP_TIMEOUT_S		120

#define TASK_PENDING(_task)		(gQMOD.tasks & (1 << (_task)))
#define TASK_SET(_task)			(gQMOD.tasks |= (1 << (_task)))
#define TASK_CLEAR(_task)		(gQMOD.tasks &= ~(1 << (_task)))


#ifndef QMOD_PSM_ACTIVE_TIME
#define QMOD_PSM_ACTIVE_TIME			2
#endif
#ifndef QMOD_PSM_PERIODIC_TIME
#define QMOD_PSM_PERIODIC_TIME			(24*3600)
#endif

#define QMOD_INFO_SIZE			24

/*
 * PRIVATE TYPES
 */

typedef enum {
	// Base state. UART off. Hopefully device in PSM
	QMOD_Step_Standby,

	// Metastates used as helpers.
	QMOD_Step_Error, // In an error state. Something has gone wrong.
	QMOD_Step_Delay, // This step delays for a while before triggering another step

	// Startup/wakeup state machines.
#if defined(QMOD_MODULE_BG95) && defined(QMOD_USE_PSM)
	QMOD_Step_Wakeup,
#endif
	QMOD_Step_Reset,
	QMOD_Step_WaitForReady,

	// Idle case. This triggers other state machines
	QMOD_Step_Idle,

	// Configuration state machine
	QMOD_Step_Configure,
	QMOD_Step_ATE0 = QMOD_Step_Configure,
	QMOD_Step_SetCMEE,
	QMOD_Step_Identify,
	QMOD_Step_GetCPIN,
#ifdef QMOD_GET_IMEI
	QMOD_Step_GetGSN,
#endif
#ifdef QMOD_GET_IMSI
	QMOD_Step_GetCIMI,
#endif
#ifdef QMOD_GET_ICCID
	QMOD_Step_GetICCID,
#endif
#ifdef QMOD_USE_NBIOT
	QMOD_Step_SetCFG_NWScan,
	QMOD_Step_SetCFG_Mode,
#endif //QMOD_USE_NBIOT
#ifdef QMOD_USE_PSM
	QMOD_Step_SetPSM,
#endif //QMOD_USE_PSM
	QMOD_Step_SetQSCLK,

	// AT state machine. Used for sync after sleep
	QMOD_Step_AT,

	// Connection state machine
	QMOD_Step_Connect,
	QMOD_Step_GetCREG = QMOD_Step_Connect,
	QMOD_Step_GetCGPADDR,

#ifdef QMOD_LOG_RSSI
	// Information state machine
	QMOD_Step_Info,
	QMOD_Step_GetRSSI = QMOD_Step_Info,
#endif

#ifdef QMOD_ENABLE_HTTP
	// Request state machines
	QMOD_Step_HTTP,
	QMOD_Step_SetURL = QMOD_Step_HTTP,
	QMOD_Step_SetURL_Content,
	QMOD_Step_SetHTTPCFG_Auth,
	QMOD_Step_SetHTTPCFG_ContentType,
	QMOD_Step_SetHTTP,
	QMOD_Step_SetHTTP_Content,
	QMOD_Step_SetHTTP_Wait,
	QMOD_Step_HTTPRead,
	QMOD_Step_HTTPRead_Content,
	QMOD_Step_HTTPRead_Ok,
	QMOD_Step_HTTPRead_Status,
	QMOD_Step_SendCallback,
#endif //QMOD_ENABLE_HTTP

} QMOD_Step_t;

typedef enum {
	QMOD_Task_Configure,
	QMOD_Task_AT,
	QMOD_Task_Connect,
#ifdef QMOD_LOG_RSSI
	QMOD_Task_Info,
#endif // QMOD_LOG_RSSI
#ifdef QMOD_ENABLE_HTTP
	QMOD_Task_HTTP,
#endif // QMOD_ENABLE_HTTP
} QMOD_Task_t;

/*
 * PRIVATE PROTOTYPES
 */

// Step execution
static void QMOD_EnterStep(QMOD_Step_t step);
static QMOD_Step_t QMOD_RunStep(QMOD_Step_t step);

#ifdef QMOD_ENABLE_HTTP
static bool QMOD_HttpRequest(const char * method, const char * url, uint8_t * tx, uint32_t tx_size, uint8_t * rx, uint32_t rx_size, QMOD_RequestCallback_t cb);
#endif

#ifdef QMOD_USE_PSM
static int QMOD_ComputePsmTimer(uint32_t seconds, bool tau_format);
#endif

static inline char * QMOD_GetInfoBuffer(QMOD_Info_t info);

/*
 * PRIVATE VARIABLES
 */

static struct {
	QMOD_Step_t step;
	QMOD_Task_t tasks;
	uint32_t retries;

	char info[QMOD_INFO_COUNT][QMOD_INFO_SIZE];

	struct {
		QMOD_Step_t next_step;
		uint32_t delay;
	} delay;

#ifdef QMOD_ENABLE_HTTP
	struct {
		const char * method;
		QMOD_RequestCallback_t callback;
		const char * url;
		uint32_t url_len;
		uint8_t * rx_bfr;
		uint32_t rx_max;
		uint8_t * tx_bfr;
		uint32_t tx_size;
		uint32_t response_size;
		uint32_t status;
	} http;

	struct {
		const char * user;
		const char * password;
	} auth;
#endif

} gQMOD;

/*
 * PUBLIC FUNCTIONS
 */

void QMOD_Init(void)
{
	GPIO_EnableOutput(QMOD_WAKE_PIN, GPIO_PIN_RESET);
#ifdef QMOD_USE_PSM
	GPIO_EnableInput(QMOD_DCD_PIN, GPIO_Pull_None);
	GPIO_EnableOutput(QMOD_DTR_PIN, GPIO_PIN_RESET);
#endif
	// The state change below should fix any other variables.
	gQMOD.tasks = 0;
	gQMOD.step = QMOD_Step_Standby;
#ifdef QMOD_ENABLE_HTTP
	gQMOD.http.callback = NULL;
	gQMOD.auth.user = NULL;
#endif
	QMOD_EnterStep(QMOD_Step_Reset);
}

void QMOD_Deinit(void)
{
	QMOD_EnterStep(QMOD_Step_Standby);
	GPIO_Deinit(QMOD_WAKE_PIN);
#ifdef QMOD_USE_PSM
	GPIO_Deinit(QMOD_DCD_PIN);
	GPIO_Deinit(QMOD_DTR_PIN);
#endif
}

void QMOD_Update(void)
{
	QMOD_Step_t step = QMOD_RunStep(gQMOD.step);
	if (step != gQMOD.step)
		QMOD_EnterStep(step);
}

void QMOD_Reset(void)
{
	QMOD_EnterStep(QMOD_Step_Reset);
}

bool QMOD_IsBusy(void)
{
	return gQMOD.step != QMOD_Step_Standby;
}

bool QMOD_IsConnected(void)
{
	return !TASK_PENDING(QMOD_Task_Connect);
}

#ifdef QMOD_ENABLE_HTTP
bool QMOD_HttpGet(const char * url, uint8_t * bfr, uint32_t bfr_size, QMOD_RequestCallback_t cb)
{
	return QMOD_HttpRequest("GET", url, NULL, 0, bfr, bfr_size, cb);
}

bool QMOD_HttpPut(const char * url, uint8_t * bfr, uint32_t bfr_size, QMOD_RequestCallback_t cb)
{
	return QMOD_HttpRequest("PUT", url, bfr, bfr_size, NULL, 0, cb);
}

bool QMOD_HttpPost(const char * url, uint8_t * tx, uint32_t tx_size, uint8_t * rx, uint32_t rx_size, QMOD_RequestCallback_t cb)
{
	return QMOD_HttpRequest("POST", url, tx, tx_size, rx, rx_size, cb);
}

bool QMOD_HttpPending(void)
{
	return TASK_PENDING(QMOD_Task_HTTP);
}

void QMOD_SetAuthentication(const char * user, const char * password)
{
	gQMOD.auth.user = user;
	gQMOD.auth.password = password;
}
#endif //QMOD_ENABLE_HTTP

void QMOD_Wakeup(void)
{
	if (gQMOD.step == QMOD_Step_Standby)
	{
		TASK_SET(QMOD_Task_AT);
#if defined(QMOD_MODULE_BG95) && defined(QMOD_USE_PSM)
		// Check for PSM signal
		bool rx_high = GPIO_Read(QMOD_DCD_PIN);
		QMOD_EnterStep(rx_high ? QMOD_Step_Idle : QMOD_Step_Wakeup);
#else
		QMOD_EnterStep(QMOD_Step_Idle);
#endif
	}
}

const char * QMOD_GetInfo(QMOD_Info_t info)
{
	if (info < QMOD_INFO_COUNT)
	{
		const char * value = QMOD_GetInfoBuffer(info);
		return value[0] ? value : NULL;
	}
	return NULL;
}


/*
 * PRIVATE FUNCTIONS
 */

static inline char * QMOD_GetInfoBuffer(QMOD_Info_t info)
{
	return gQMOD.info[info];
}

#ifdef QMOD_ENABLE_HTTP
static bool QMOD_HttpRequest(const char * method, const char * url, uint8_t * tx, uint32_t tx_size, uint8_t * rx, uint32_t rx_size, QMOD_RequestCallback_t cb)
{
	QMOD_Wakeup();

	if (QMOD_HttpPending())
		// Already a pending request. This request cannot be executed.
		return false;

	TASK_SET(QMOD_Task_HTTP);

	gQMOD.http.method = method;
	gQMOD.http.url = url;
	gQMOD.http.tx_bfr = tx;
	gQMOD.http.tx_size = tx_size;
	gQMOD.http.rx_bfr = rx;
	gQMOD.http.rx_max = rx_size;
	gQMOD.http.callback = cb;
	gQMOD.http.url_len = strlen(url);
	return true;
}
#endif // QMOD_ENABLE_HTTP

static bool QMOD_HandleURC(const char * line)
{
	if (strncmp("+QIURC:", line, 7) == 0)
		// Discard +QIURC:
		return true;
	return false;
}

static void QMOD_ClearData(void)
{
	bzero(gQMOD.info, sizeof(gQMOD.info));
}

static void QMOD_EnterStep(QMOD_Step_t step)
{
	if (step != gQMOD.step)
	{
		if (step != QMOD_Step_Delay && gQMOD.step != QMOD_Step_Delay)
			// Reset retries when changing step. Going to and from a delay
			gQMOD.retries = 0;

		// During the step change, we may want the UART on/off.
		if (gQMOD.step == QMOD_Step_Standby)
		{
			GPIO_Reset(QMOD_DTR_PIN);
#ifdef QMOD_LOG_RSSI
			TASK_SET(QMOD_Task_Info);
#endif
			AT_Init();
			AT_SetUrcHandler(QMOD_HandleURC);
		}
		else if (step == QMOD_Step_Standby)
		{
			GPIO_Set(QMOD_DTR_PIN);
			AT_Deinit();
		}

		if (step == QMOD_Step_Reset)
		{
			QMOD_ClearData();
			TASK_SET(QMOD_Task_Configure);
			TASK_SET(QMOD_Task_Connect);
		}

		AT_StartCommand();

		// Might as well notify everyone of an error.
		if (step == QMOD_Step_Error)
			Log_Error("Error %u", gQMOD.step);

		gQMOD.step = step;
	}
}

static QMOD_Step_t QMOD_DelayStep(QMOD_Step_t next_step, uint32_t delay)
{
	gQMOD.delay.next_step = next_step;
	gQMOD.delay.delay = delay;
	return QMOD_Step_Delay;
}

static QMOD_Step_t QMOD_RetryStep(QMOD_Step_t step, uint32_t delay, uint32_t retries)
{
	if (gQMOD.retries < retries)
	{
		gQMOD.retries += 1;
		return QMOD_DelayStep(step, delay);
	}
	return QMOD_Step_Error;
}

static QMOD_Step_t QMOD_RunStep(QMOD_Step_t step)
{
	AT_Status_t r;

	switch (step)
	{

	/*
	 * UTILITY STATE MACHINE STEPS
	 */

	default:
	case QMOD_Step_Standby:
		return step;

	case QMOD_Step_Error:
		// After any error, we should probably try to AT to resync.
		TASK_SET(QMOD_Task_AT);
#ifdef QMOD_ENABLE_HTTP
		if (TASK_PENDING(QMOD_Task_HTTP))
		{
			// There is a pending HTTP transaction.
			// We need to guarantee the user gets their callback.
			gQMOD.http.status = 0;
			gQMOD.http.response_size = 0;
			return QMOD_Step_SendCallback;
		}
#endif // QMOD_ENABLE_HTTP
		return QMOD_Step_Standby;

	case QMOD_Step_Delay:
		AT_SetTimeout(gQMOD.delay.delay);
		if (AT_TimeoutElapsed())
			return gQMOD.delay.next_step;
		return step;


#if defined(QMOD_MODULE_BG95) && defined(QMOD_USE_PSM)

	/*
	 * WAKEUP STATE MACHINE
	 * 		A speciality wakeup step for the BG95
	 */

	case QMOD_Step_Wakeup:
		AT_SetTimeout(100);
		if (!AT_TimeoutElapsed())
		{
			GPIO_Set(QMOD_WAKE_PIN);
			r = AT_Pending;
		}
		else
		{
			GPIO_Reset(QMOD_WAKE_PIN);
			return QMOD_Step_WaitForReady;
		}
		break;

#endif

	/*
	 * RESET STATE MACHINE
	 * 		Perform device wake process, and await APP RDY notification
	 * 		The idle state will then kick off other tasks as need be
	 */

#if defined(QMOD_MODULE_BG95)
	case QMOD_Step_Reset:
		AT_SetTimeout(2500);
		if (!AT_TimeoutElapsed())
		{
			// Hold reset for 3 seconds to boot modem.
			GPIO_Set(QMOD_WAKE_PIN);
			r = AT_Pending;
		}
		else
		{
			GPIO_Reset(QMOD_WAKE_PIN);
			r = AT_Ok;
		}
		break;

	case QMOD_Step_WaitForReady:
		AT_SetTimeout(12000);
		r = AT_ExpectMatch("APP RDY");
		if (r == AT_Unexpected)
			return step;
		break;

#elif defined(QMOD_MODULE_EG800Q)
	case QMOD_Step_Reset:
		AT_SetTimeout(900);
		uint32_t remaining = AT_GetTimeout();
		r = AT_Pending;
		if (remaining > 550) // 350ms, reset active
		{
			GPIO_Set(QMOD_RST_PIN);
		}
		else if (remaining > 0) // 550ms wake active
		{
			GPIO_Set(QMOD_WAKE_PIN);
			GPIO_Reset(QMOD_RST_PIN);
		}
		else
		{
			GPIO_Set(QMOD_WAKE_PIN);
			GPIO_Set(QMOD_RST_PIN);
			r = AT_Ok;
		}
		break;

	case QMOD_Step_WaitForReady:
		AT_SetTimeout(5000);
		r = AT_ExpectMatch("RDY");
		if (r == AT_Unexpected)
			return step;
		if (r == AT_Ok)
			return QMOD_DelayStep(QMOD_Step_Idle, 1000);
		break;
#endif

	case QMOD_Step_Idle:
		if (TASK_PENDING(QMOD_Task_Configure))
			return QMOD_Step_Configure;
		if (TASK_PENDING(QMOD_Task_AT))
			return QMOD_Step_AT;
		if (TASK_PENDING(QMOD_Task_Connect))
			return QMOD_Step_Connect;
#ifdef QMOD_ENABLE_HTTP
		if (TASK_PENDING(QMOD_Task_HTTP))
			return QMOD_Step_HTTP;
#endif //QMOD_ENABLE_HTTP
#ifdef QMOD_LOG_RSSI
		if (TASK_PENDING(QMOD_Task_Info))
			// Try to do the HTTP first.
			// We dont get a sane RSSI value wihout some comms.
			return QMOD_Step_Info;
#endif
		// Nothing else to do.
		return QMOD_Step_Standby;

	/*
	 * CONFIGURATION STATE MACHINE
	 * 		Interrogates modem and performs "one-time" configuration.
	 */

	case QMOD_Step_ATE0:
		AT_Command("E0");
		r = AT_ExpectOk();
		if (r == AT_Unexpected)
			return step;
		break;

	case QMOD_Step_SetCMEE:
		// Disable error codes. It simplifies the API to the AT module.
		AT_Command("+CMEE=0");
		r = AT_ExpectOk();
		break;

	case QMOD_Step_Identify:
		AT_Command("+GMM");
		r = AT_ExpectResponsef(1, "%s", QMOD_GetInfoBuffer(QMOD_Info_Model));
		if (r == AT_Ok)
			Log_Info("detected %s", QMOD_GetInfoBuffer(QMOD_Info_Model));
		break;

	case QMOD_Step_GetCPIN:
		AT_Command("+CPIN?");
		r = AT_ExpectResponse("+CPIN: READY");
		if (r != AT_Ok && r != AT_Pending)
		{
			Log_Warn("SIM detect error");
			return QMOD_RetryStep(step, 500, 2);
		}
		break;

#ifdef QMOD_GET_IMEI
	case QMOD_Step_GetGSN:
		AT_Command("+GSN");
		r = AT_ExpectResponsef(1, "%[0-9]", QMOD_GetInfoBuffer(QMOD_Info_IMEI));
		if (r == AT_Ok)
			Log_Info("IMEI %s", QMOD_GetInfoBuffer(QMOD_Info_IMEI));
		break;
#endif
#ifdef QMOD_GET_IMSI
	case QMOD_Step_GetCIMI:
		AT_Command("+CIMI");
		r = AT_ExpectResponsef(1, "%[0-9]", QMOD_GetInfoBuffer(QMOD_Info_IMSI));
		if (r == AT_Ok)
			Log_Info("IMSI %s", QMOD_GetInfoBuffer(QMOD_Info_IMSI));
		break;
#endif
#ifdef QMOD_GET_ICCID
	case QMOD_Step_GetICCID:
		AT_Command("+ICCID");
		r = AT_ExpectResponsef(1, "+ICCID: %[0-9]", QMOD_GetInfoBuffer(QMOD_Info_ICCID));
		if (r == AT_Ok)
			Log_Info("ICCID %s", QMOD_GetInfoBuffer(QMOD_Info_ICCID));
		break;
#endif

#ifdef QMOD_USE_NBIOT
	case QMOD_Step_SetCFG_NWScan:

		// Skip these steps on modems that dont support NBIOT
		//if (gQMOD.model <= QMOD_Model_M1) { return step + 2; }

		// Scan for NBIOT then eMTC
		AT_Command("+QCFG=\"nwscanseq\",0302");
		r = AT_ExpectOk();
		break;

	case QMOD_Step_SetCFG_Mode:
		// Scan for NBIOT then eMTC
		AT_Command("+QCFG=\"iotopmode\",2,1");
		r = AT_ExpectOk();
		break;
#endif // QMOD_USE_NBIOT

#ifdef QMOD_USE_PSM
	case QMOD_Step_SetPSM:
		// PSM mode supported. Set the timers.
		AT_Commandf("+QPSMS=1,,,\"%08X\",\"%08X\"",
				QMOD_ComputePsmTimer(QMOD_PSM_PERIODIC_TIME, true),  // T3412, Periodic TAU
				QMOD_ComputePsmTimer(QMOD_PSM_ACTIVE_TIME, false)  // T3412, Active time
				);
		r = AT_ExpectOk();
		break;
#endif

	case QMOD_Step_SetQSCLK:
#ifdef QMOD_USE_PSM
		AT_Command("+QSCLK=1");
#else
		AT_Command("+QSCLK=0");
#endif
		r = AT_ExpectOk();
		if (r == AT_Ok)
		{
			TASK_CLEAR(QMOD_Task_Configure);
			return QMOD_Step_Idle;
		}
		break;

	/*
	 * AT STATE MACHINE
	 * 		Check our serial is synchronised
	 */

	case QMOD_Step_AT:
		AT_Command("");
		r = AT_ExpectOk();
		if (r == AT_Ok)
		{
			TASK_CLEAR(QMOD_Task_AT);
			return QMOD_Step_Idle;
		}
		else if (r != AT_Pending)
		{
			// 2 retries, 500ms spacing.
			return QMOD_RetryStep(step, 500, 2);
		}
		break;

	/*
	 * CONNECTION STATE MACHINE
	 * 		Wait for a valid network connection.
	 */

	case QMOD_Step_GetCREG:
		AT_Command("+CEREG?");
		int status;
		r = AT_ExpectResponsef(1, "+CEREG: 0,%d", &status);
		if (r == AT_Ok)
		{
			switch (status)
			{
			case 1: // Registered
			case 5: // Roaming
				Log_Info("registered");
				break;
			case 2: // Searching
				Log_Info("searching...");
				// Check again in 3 seconds.
				return QMOD_RetryStep(step, 3000, 100);
			case 0: // Not searching
				Log_Warn("search stopped");
				return QMOD_Step_Error;
			default:
			case 3:  // Resistration denied
				Log_Warn("registration denied");
				return QMOD_Step_Error;
			}
		}
		else if (r == AT_Unexpected)
		{
			// 100 retries, 5 minutes
			return QMOD_RetryStep(step, 3000, 100);
		}
		break;

	case QMOD_Step_GetCGPADDR:
		AT_Command("+CGPADDR=1");
#if defined(QMOD_MODULE_BG95)
		r = AT_ExpectResponsef(1, "+CGPADDR: 1,%s", QMOD_GetInfoBuffer(QMOD_Info_IP));
#else
		r = AT_ExpectResponsef(1, "+CGPADDR: 1,\"%[^\"]\"", QMOD_GetInfoBuffer(QMOD_Info_IP));
#endif
		if (r == AT_Ok)
		{
			TASK_CLEAR(QMOD_Task_Connect);
			Log_Info("ip %s", QMOD_GetInfoBuffer(QMOD_Info_IP));
			return QMOD_Step_Idle;
		}
		else if (r == AT_Unexpected)
		{
			Log_Info("waiting on ip...");
			// 20 retries, 1 minute
			return QMOD_RetryStep(step, 3000, 20);
		}
		break;

#ifdef QMOD_LOG_RSSI
	/*
	 * INFO STATE MACHINE
	 * 		Gets information after an HTTP transaction.
	 */

	case QMOD_Step_GetRSSI:
		AT_Command("+QCSQ");
		int rssi = 0;
		char mode[16];
		r = AT_ExpectResponsef(1, "+QCSQ: \"%[^\"]\",%d", mode, &rssi);
		if (r == AT_Ok)
		{
			TASK_CLEAR(QMOD_Task_Info);
			Log_Info("%s: %d dBm", mode, rssi);
			return QMOD_Step_Idle;
		}
		break;
#endif //QMOD_LOG_RSSI

#ifdef QMOD_ENABLE_HTTP

	/*
	 * HTTP STATE MACHINE
	 * 		Configures and performs a standard HTTP request.
	 */

	case QMOD_Step_SetURL:
		AT_Commandf("+QHTTPURL=%u", (int)gQMOD.http.url_len);
		r = AT_ExpectMatch("CONNECT");
		break;

	case QMOD_Step_SetURL_Content:
		AT_CommandRaw((uint8_t*)gQMOD.http.url, gQMOD.http.url_len);
		AT_SetTimeout(3000);
		r = AT_ExpectOk();
		break;

	case QMOD_Step_SetHTTPCFG_Auth:
		if (gQMOD.auth.user == NULL)
			return step + 1; // No user. Skip this step.
		AT_Commandf("+QHTTPCFG=\"auth\",\"%s:%s\"", gQMOD.auth.user, gQMOD.auth.password);
		r = AT_ExpectOk();
		break;

	case QMOD_Step_SetHTTPCFG_ContentType:
		AT_Command("+QHTTPCFG=\"contenttype\",2"); // application/octet-steam
		r = AT_ExpectOk();
		break;

	case QMOD_Step_SetHTTP:
		if (gQMOD.http.method[0] == 'G')
		{
			// Get request. No data.
			AT_Commandf("+QHTTP%s=%u",  gQMOD.http.method, QMOD_HTTP_TIMEOUT_S);
			r = AT_ExpectOk();
			if (r == AT_Ok)
				return QMOD_Step_SetHTTP_Wait;
		}
		else
		{
			// PUT or POST request.
			// Data is expected.
			AT_Commandf("+QHTTP%s=%u,%u,%u", gQMOD.http.method, (int)gQMOD.http.tx_size, 5, QMOD_HTTP_TIMEOUT_S);

			// Connection is established before the CONNECT comes though. WE need to wait for it.
			AT_SetTimeout((QMOD_HTTP_TIMEOUT_S + 1) * 1000);
			r = AT_ExpectMatch("CONNECT");
		}
		break;

	case QMOD_Step_SetHTTP_Content:
		AT_CommandRaw((uint8_t*)gQMOD.http.tx_bfr, gQMOD.http.tx_size);
		AT_SetTimeout(6000);
		r = AT_ExpectOk();
		break;

	case QMOD_Step_SetHTTP_Wait:
		AT_SetTimeout((QMOD_HTTP_TIMEOUT_S + 1) * 1000);
		int response_code = 0, response_status = 0, response_size = 0;
		// This matches +QHTTP<method>:
		r = AT_ExpectMatchf(1, "+QHTTP%*[^:]: %u,%u,%u",
			&response_code, &response_status, &response_size
		);
		if (r == AT_Ok)
		{
			gQMOD.http.status = response_status; // These parameters may not be recieved.
			gQMOD.http.response_size = response_size;

			if (response_code != 0)
			{
				// This could be an HTTP failure? Not necessarily a connection issue.
				Log_Error("http failure");
				return QMOD_Step_SendCallback;
			}

			Log_Info("%s %u (%u bytes)", gQMOD.http.method, gQMOD.http.status, gQMOD.http.response_size);

			if (gQMOD.http.rx_max == 0 || gQMOD.http.response_size == 0)
				return QMOD_Step_SendCallback; // No data

			if (gQMOD.http.rx_max < gQMOD.http.response_size)
			{
				// We cant read this.
				Log_Error("rx buffer (%d) too small for payload (%d)", gQMOD.http.rx_max, gQMOD.http.response_size);
				gQMOD.http.response_size = 0;
				return QMOD_Step_SendCallback;
			}

			// Lets read the request.
			return QMOD_Step_HTTPRead;
		}
		break;

	case QMOD_Step_HTTPRead:
		AT_Command("+QHTTPREAD");
		r = AT_ExpectMatch("CONNECT");
		break;

	case QMOD_Step_HTTPRead_Content:
		// WARN: If more content is returned than can be parsed, this will gum up later requests....
		AT_SetTimeout(5000);
		r = AT_ExpectRaw(gQMOD.http.rx_bfr, gQMOD.http.response_size);
		break;

	case QMOD_Step_HTTPRead_Ok:
		r = AT_ExpectOk();
		break;

	case QMOD_Step_HTTPRead_Status:
		int http_read_status;
		r = AT_ExpectMatchf(1, "+QHTTPREAD: %d", &http_read_status);
		if (r == AT_Ok && http_read_status != 0)
		{
			Log_Error("Error during http read");
			return QMOD_Step_Error;
		}
		break;

	case QMOD_Step_SendCallback:
		TASK_CLEAR(QMOD_Task_HTTP);
		if (gQMOD.http.callback)
		{
			gQMOD.http.callback(gQMOD.http.status, gQMOD.http.response_size);

			if (gQMOD.step != QMOD_Step_SendCallback)
			{
				// The user issued a state change within the callback.
				// Do not intefere with the new state change.
				return gQMOD.step;
			}
		}
		return QMOD_Step_Idle;

#endif // QMOD_ENABLE_HTTP

	}

	// Default cases top simplify unhandled cases above.
	switch (r)
	{
	case AT_Pending:
		return step;
	case AT_Ok:
		return ++step;
	default:
		return QMOD_Step_Error;
	}
}


#ifdef QMOD_USE_PSM
static int QMOD_ComputePsmTimer(uint32_t seconds, bool tau_format)
{
	uint8_t timer;
	// First, figure out the lowest base we can use.

	if (tau_format)
	{
		if (seconds <= 0x1F * 2)
			// 2 second timers
			timer = (3 << 5) | (seconds / 2);
		else if (seconds <= 0x1F * 30)
			// 30 second timer
			timer = (4 << 5) | (seconds / 30);
		else if (seconds <= 0x1F * 60)
			// 60 second timer
			timer = (5 << 5) | (seconds / 60);
		else if (seconds <= 0x1F * (10 * 60))
			// 10 minute timer
			timer = (0 << 5) | (seconds / (10 * 60));
		else if (seconds <= 0x1F * (60 * 60))
			// 1 hour blocks
			timer = (1 << 5) | (seconds / (60 * 60));
		else
			// 10 hour blocks
			timer = (2 << 5) | (seconds / (10 * 60 * 60));
	}
	else // Active time format
	{
		if (seconds <= 0x1F * 2)
			// 2 second timers
			timer = (0 << 5) | (seconds / 2);
		else if (seconds <= 0x1F * 60)
			// 1 minute timer
			timer = (1 << 5) | (seconds / 60);
		else
			// 6 minute timer (deci-hours)
			timer = (2 << 5) | (seconds / (6 * 60));
	}

	// This needs to be formatted as an 8 bit string by the caller. If we pack this wierdly, then we can print it with %08X
	// 0b01100001 -> 0x01100001
	uint32_t bhex = 0;
	for (uint32_t i = 0; i < 8; i++)
	{
		bhex |= ((timer >> i) & 1) << (i*4);
	}
	return bhex;
}
#endif //QMOD_USE_PSM


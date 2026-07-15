#include "BG95.h"

#include "Core.h"
#include "GPIO.h"
#include "AT.h"
#include "Logging.h"

#include <stdio.h>
#include <stdarg.h>

/*
 * PRIVATE DEFINITIONS
 */

#define LOG_SOURCE "BG95"
#define BG95_HTTP_TIMEOUT_S		120

#define TASK_PENDING(_task)		(gBG95.tasks & (_task))
#define TASK_SET(_task)			(gBG95.tasks |= (_task))
#define TASK_CLEAR(_task)		(gBG95.tasks &= ~(_task))

//#define BG95_NBIOT
//#define BG95_LOG_RSSI
//#define BG95_USE_PSM

#ifndef BG95_PSM_ACTIVE_TIME
#define BG95_PSM_ACTIVE_TIME			2
#endif
#ifndef BG95_PSM_PERIODIC_TIME
#define BG95_PSM_PERIODIC_TIME			(24*3600)
#endif

/*
 * PRIVATE TYPES
 */

typedef enum {
	// Base state. UART off. Hopefully device in PSM
	BG95_Step_Standby,

	// Metastates used as helpers.
	BG95_Step_Error, // In an error state. Something has gone wrong.
	BG95_Step_Delay, // This step delays for a while before triggering another step

	// Startup/wakeup state machines.
	BG95_Step_Wakeup,
	BG95_Step_Reset,
	BG95_Step_WaitForReady,
	BG95_Step_ATE0,
	BG95_Step_AT,
	BG95_Step_Idle,

	// Configuration state machine
	BG95_Step_Configure,
	BG95_Step_SetCMEE = BG95_Step_Configure,
	BG95_Step_Identify,
	BG95_Step_GetCPIN,
	BG95_Step_GetGSN,
	BG95_Step_GetCIMI,
#ifdef BG95_NBIOT
	BG95_Step_SetCFG_NWScan,
	BG95_Step_SetCFG_Mode,
#endif //BG95_NBIOT
	BG95_Step_SetPSM,

	// Connection state machine
	BG95_Step_Connect,
	BG95_Step_GetCREG = BG95_Step_Connect,
	BG95_Step_GetCGPADDR,

#ifdef BG95_LOG_RSSI
	// Information state machine
	BG95_Step_Info,
	BG95_Step_GetRSSI = BG95_Step_Info,
#endif

	// Request state machines
	BG95_Step_HTTP,
	BG95_Step_SetURL = BG95_Step_HTTP,
	BG95_Step_SetURL_Content,
	BG95_Step_SetHTTPCFG_Auth,
	BG95_Step_SetHTTPCFG_ContentType,
	BG95_Step_SetHTTP,
	BG95_Step_SetHTTP_Content,
	BG95_Step_SetHTTP_Wait,
	BG95_Step_HTTPRead,
	BG95_Step_HTTPRead_Content,
	BG95_Step_HTTPRead_Ok,
	BG95_Step_HTTPRead_Status,
	BG95_Step_SendCallback,
} BG95_Step_t;

typedef enum {
	BG95_Task_Configure = 1 << 0,
	BG95_Task_Connect   = 1 << 1,
#ifdef BG95_LOG_RSSI
	BG95_Task_Info		= 1 << 2,
#endif // BG95_LOG_RSSI
	BG95_Task_HTTP		= 1 << 3,
} BG95_Task_t;

/*
 * PRIVATE PROTOTYPES
 */

// Step execution
static void BG95_EnterStep(BG95_Step_t step);
static BG95_Step_t BG95_RunStep(BG95_Step_t step);

static bool BG95_HttpRequest(const char * method, const char * url, uint8_t * tx, uint32_t tx_size, uint8_t * rx, uint32_t rx_size, BG95_RequestCallback_t cb);

#ifdef BG95_USE_PSM
static int BG95_ComputePsmTimer(uint32_t seconds, bool tau_format);
#endif

/*
 * PRIVATE VARIABLES
 */

static struct {
	BG95_Step_t step;
	BG95_Task_t tasks;
	uint32_t retries;

	char imei[20];
	char imsi[20];
	uint32_t ip;
	BG95_Model_t model;

	struct {
		BG95_Step_t next_step;
		uint32_t delay;
	} delay;

	struct {
		const char * method;
		BG95_RequestCallback_t callback;
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

} gBG95;

/*
 * PUBLIC FUNCTIONS
 */

void BG95_Init(void)
{
	GPIO_EnableOutput(BG95_WAKE_PIN, GPIO_PIN_RESET);
#ifdef BG95_USE_PSM
	GPIO_EnableInput(BG95_PSM_PIN, GPIO_Pull_None);
#endif
	// The state change below should fix any other variables.
	gBG95.tasks = 0;
	gBG95.step = BG95_Step_Standby;
	gBG95.http.callback = NULL;
	gBG95.auth.user = NULL;
	BG95_EnterStep(BG95_Step_Reset);
}

void BG95_Deinit(void)
{
	BG95_EnterStep(BG95_Step_Standby);
	GPIO_Deinit(BG95_WAKE_PIN);
#ifdef BG95_USE_PSM
	GPIO_Deinit(BG95_PSM_PIN);
#endif
}

void BG95_Update(void)
{
	BG95_Step_t step = BG95_RunStep(gBG95.step);
	if (step != gBG95.step)
	{
		BG95_EnterStep(step);
	}
}

void BG95_Reset(void)
{
	BG95_EnterStep(BG95_Step_Reset);
}

bool BG95_IsBusy(void)
{
	return gBG95.step != BG95_Step_Standby;
}

bool BG95_IsConnected(void)
{
	return !TASK_PENDING(BG95_Task_Connect);
}

bool BG95_HttpGet(const char * url, uint8_t * bfr, uint32_t bfr_size, BG95_RequestCallback_t cb)
{
	return BG95_HttpRequest("GET", url, NULL, 0, bfr, bfr_size, cb);
}

bool BG95_HttpPut(const char * url, uint8_t * bfr, uint32_t bfr_size, BG95_RequestCallback_t cb)
{
	return BG95_HttpRequest("PUT", url, bfr, bfr_size, NULL, 0, cb);
}

bool BG95_HttpPost(const char * url, uint8_t * tx, uint32_t tx_size, uint8_t * rx, uint32_t rx_size, BG95_RequestCallback_t cb)
{
	return BG95_HttpRequest("POST", url, tx, tx_size, rx, rx_size, cb);
}

bool BG95_HttpPending(void)
{
	return TASK_PENDING(BG95_Task_HTTP);
}

void BG95_Wakeup(void)
{
	if (gBG95.step == BG95_Step_Standby)
	{
#ifdef BG95_USE_PSM
		// Check for PSM signal
		bool rx_high = GPIO_Read(BG95_PSM_PIN);
		BG95_EnterStep(rx_high ? BG95_Step_AT : BG95_Step_Wakeup);
#else
		BG95_EnterStep(BG95_Step_AT);
#endif //BG95_USE_PSM
	}
}

BG95_Model_t BG95_GetModel(void)
{
	return gBG95.model;
}

const char * BG95_GetIMSI(void)
{
	return gBG95.imsi[0] ? gBG95.imsi : NULL;
}

const char * BG95_GetIMEI(void)
{
	return gBG95.imei[0] ? gBG95.imei : NULL;
}

const uint8_t * BG95_GetIP(void)
{
	if (gBG95.ip != 0)
	{
		return (uint8_t*)&gBG95.ip;
	}
	return NULL;
}

void BG95_SetAuthentication(const char * user, const char * password)
{
	gBG95.auth.user = user;
	gBG95.auth.password = password;
}

/*
 * PRIVATE FUNCTIONS
 */

static bool BG95_HttpRequest(const char * method, const char * url, uint8_t * tx, uint32_t tx_size, uint8_t * rx, uint32_t rx_size, BG95_RequestCallback_t cb)
{
	BG95_Wakeup();

	if (BG95_HttpPending())
	{
		// Already a pending request. This request cannot be executed.
		return false;
	}

	TASK_SET(BG95_Task_HTTP);

	gBG95.http.method = method;
	gBG95.http.url = url;
	gBG95.http.tx_bfr = tx;
	gBG95.http.tx_size = tx_size;
	gBG95.http.rx_bfr = rx;
	gBG95.http.rx_max = rx_size;
	gBG95.http.callback = cb;
	gBG95.http.url_len = strlen(url);
	return true;
}

static bool BG95_HandleURC(const char * line)
{
	if (strncmp("+QIURC:", line, 7) == 0)
	{
		// Discard +QIURC:
		return true;
	}
	return false;
}

static void BG95_ClearData(void)
{
	gBG95.imsi[0] = 0;
	gBG95.imei[0] = 0;
	gBG95.model = BG95_Model_None;
	gBG95.ip = 0;
}

static void BG95_EnterStep(BG95_Step_t step)
{
	if (step != gBG95.step)
	{
		gBG95.retries = 0;

		// During the step change, we may want the UART on/off.
		if (gBG95.step == BG95_Step_Standby)
		{
#ifdef BG95_LOG_RSSI
			TASK_SET(BG95_Task_Info);
#endif
			AT_Init();
			AT_SetUrcHandler(BG95_HandleURC);
		}
		else if (step == BG95_Step_Standby)
		{
			AT_Deinit();
		}

		if (step == BG95_Step_Reset)
		{
			BG95_ClearData();
			TASK_SET(BG95_Task_Configure);
			TASK_SET(BG95_Task_Connect);
		}

		AT_StartCommand();

		// Might as well notify everyone of an error.
		if (step == BG95_Step_Error)
		{
			Log_Error("Error %u", gBG95.step);
		}

		gBG95.step = step;
	}
}

static BG95_Step_t BG95_DelayStep(BG95_Step_t next_step, uint32_t delay)
{
	gBG95.delay.next_step = next_step;
	gBG95.delay.delay = delay;
	return BG95_Step_Delay;
}

static BG95_Step_t BG95_RetryStep(BG95_Step_t step, uint32_t delay, uint32_t retries)
{
	if (gBG95.retries < retries)
	{
		gBG95.retries += 1;
		return BG95_DelayStep(step, delay);
	}
	return BG95_Step_Error;
}

static BG95_Step_t BG95_RunStep(BG95_Step_t step)
{
	AT_Status_t r;

	switch (step)
	{

	/*
	 * UTILITY STATE MACHINE STEPS
	 */

	default:
	case BG95_Step_Standby:
		return step;

	case BG95_Step_Error:

		if (TASK_PENDING(BG95_Task_HTTP))
		{
			// There is a pending HTTP transaction.
			// We need to guarantee the user gets their callback.
			gBG95.http.status = 0;
			gBG95.http.response_size = 0;
			return BG95_Step_SendCallback;
		}
		return BG95_Step_Standby;

	case BG95_Step_Delay:
		AT_SetTimeout(gBG95.delay.delay);
		if (AT_GetTimeout())
		{
			return gBG95.delay.next_step;
		}
		return step;

	/*
	 * RESET & WAKEUP STATE MACHINE
	 * 		Perform device wake process, and await APP RDY notification
	 * 		The idle state will then kick off other tasks as need be
	 */

	case BG95_Step_Wakeup:
	case BG95_Step_Reset:
		AT_SetTimeout(step == BG95_Step_Reset ? 2500 : 100);
		if (!AT_GetTimeout())
		{
			// Hold reset for 3 seconds to boot modem.
			GPIO_Set(BG95_WAKE_PIN);
			return step;
		}
		else
		{
			GPIO_Reset(BG95_WAKE_PIN);
			return BG95_Step_WaitForReady;
		}

	case BG95_Step_WaitForReady:
		AT_SetTimeout(12000);
		r = AT_ExpectMatch("APP RDY");
		if (r == AT_Unexpected) { return step; }
		break;

	case BG95_Step_ATE0:
		AT_Command("E0");
		r = AT_ExpectOk();
		if (r == AT_Unexpected) { return step; }
		break;

	case BG95_Step_AT:
		AT_Command("");
		r = AT_ExpectOk();
		break;

	case BG95_Step_Idle:
		if (TASK_PENDING(BG95_Task_Configure))
		{
			return BG95_Step_Configure;
		}
		if (TASK_PENDING(BG95_Task_Connect))
		{
			return BG95_Step_Connect;
		}
		if (TASK_PENDING(BG95_Task_HTTP))
		{
			return BG95_Step_HTTP;
		}
#ifdef BG95_LOG_RSSI
		if (TASK_PENDING(BG95_Task_Info))
		{
			// Try to do the HTTP first.
			// We dont get a sane RSSI value wihout some comms.
			return BG95_Step_Info;
		}
#endif
		// Nothing else to do.
		return BG95_Step_Standby;


	/*
	 * CONFIGURATION STATE MACHINE
	 * 		Interrogates modem and performs "one-time" configuration.
	 * 		This also awaits network connection.
	 */

	case BG95_Step_SetCMEE:
		// Disable error codes. It simplifies the API to the AT module.
		AT_Command("+CMEE=0");
		r = AT_ExpectOk();
		break;

	case BG95_Step_Identify:
		AT_Command("+GMM");
		int model;
		r = AT_ExpectResponsef(1, "BG95-M%d", &model);
		if (r == AT_Ok)
		{
			gBG95.model = BG95_Model_M1 - 1 + model;
			Log_Info("detected BG95M%d", model);
		}
		break;

	case BG95_Step_GetCPIN:
		AT_Command("+CPIN?");
		r = AT_ExpectResponse("+CPIN: READY");
		if (r != AT_Ok && r != AT_Pending)
		{
			Log_Warn("SIM detect error");
		}
		break;

	case BG95_Step_GetGSN:
		AT_Command("+GSN");
		r = AT_ExpectResponsef(1, "%[0-9]", gBG95.imei);
		if (r == AT_Ok)
		{
			Log_Info("IMEI %s", gBG95.imei);
		}
		break;

	case BG95_Step_GetCIMI:
		AT_Command("+CIMI");
		r = AT_ExpectResponsef(1, "%[0-9]", gBG95.imsi);
		if (r == AT_Ok)
		{
			Log_Info("IMSI %s", gBG95.imsi);
		}
		break;

#ifdef BG95_NBIOT
	case BG95_Step_SetCFG_NWScan:

		// Skip these steps on modems that dont support NBIOT
		if (gBG95.model <= BG95_Model_M1) { return step + 2; }

		// Scan for NBIOT then eMTC
		AT_Command("+QCFG=\"nwscanseq\",0302");
		r = AT_ExpectOk();
		break;

	case BG95_Step_SetCFG_Mode:
		// Scan for NBIOT then eMTC
		AT_Command("+QCFG=\"iotopmode\",2,1");
		r = AT_ExpectOk();
		break;
#endif // BG95_NBIOT

	case BG95_Step_SetPSM:
#ifdef BG95_USE_PSM
		// PSM mode supported. Set the timers.
		AT_Commandf("+QPSMS=1,,,\"%08X\",\"%08X\"",
				BG95_ComputePsmTimer(BG95_PSM_PERIODIC_TIME, true),  // T3412, Periodic TAU
				BG95_ComputePsmTimer(BG95_PSM_ACTIVE_TIME, false)  // T3412, Active time
				);
#else
		// PSM mode not supported. Disable it.
		AT_Command("+QPSMS=0");
#endif //BG95_USE_PSM
		r = AT_ExpectOk();
		if (r == AT_Ok)
		{
			TASK_CLEAR(BG95_Task_Configure);
			return BG95_Step_Idle;
		}
		break;

	/*
	 * CONNECTION STATE MACHINE
	 * 		Wait for a valid network connection.
	 */

	case BG95_Step_GetCREG:
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
				return BG95_RetryStep(step, 3000, 100);
			case 0: // Not searching
				Log_Warn("search stopped");
				return BG95_Step_Error;
			default:
			case 3:  // Resistration denied
				Log_Warn("registration denied");
				return BG95_Step_Error;
			}
		}
		else if (r == AT_Unexpected)
		{
			// 100 retries, 5 minutes
			return BG95_RetryStep(step, 3000, 100);
		}
		break;

	case BG95_Step_GetCGPADDR:
		int ip[4] = {0};
		AT_Command("+CGPADDR=1");
		r = AT_ExpectResponsef(4, "+CGPADDR: 1,%d.%d.%d.%d", ip+0, ip+1, ip+2, ip+3);
		if (r == AT_Ok)
		{
			TASK_CLEAR(BG95_Task_Connect);
			Log_Info("ip %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
			((uint8_t*)&gBG95.ip)[0] = ip[0];
			((uint8_t*)&gBG95.ip)[1] = ip[1];
			((uint8_t*)&gBG95.ip)[2] = ip[2];
			((uint8_t*)&gBG95.ip)[3] = ip[3];
			return BG95_Step_Idle;
		}
		else if (r == AT_Unexpected)
		{
			Log_Info("waiting on ip...");
			// 20 retries, 1 minute
			return BG95_RetryStep(step, 3000, 20);
		}
		break;

#ifdef BG95_LOG_RSSI
	/*
	 * INFO STATE MACHINE
	 * 		Gets information after an HTTP transaction.
	 */

	case BG95_Step_GetRSSI:
		AT_Command("+QCSQ");
		int rssi = 0;
		char mode[16];
		r = AT_ExpectResponsef(1, "+QCSQ: \"%[^\"]\",%d", mode, &rssi);
		if (r == AT_Ok)
		{
			TASK_CLEAR(BG95_Task_Info);
			Log_Info("%s: %d dBm", mode, rssi);
			return BG95_Step_Idle;
		}
		break;
#endif //BG95_LOG_RSSI

	/*
	 * HTTP STATE MACHINE
	 * 		Configures and performs a standard HTTP request.
	 */

	case BG95_Step_SetURL:
		AT_Commandf("+QHTTPURL=%u", (int)gBG95.http.url_len);
		r = AT_ExpectMatch("CONNECT");
		break;

	case BG95_Step_SetURL_Content:
		AT_CommandRaw((uint8_t*)gBG95.http.url, gBG95.http.url_len);
		r = AT_ExpectOk();
		break;

	case BG95_Step_SetHTTPCFG_Auth:
		if (gBG95.auth.user == NULL)
			return step + 1; // No user. Skip this step.
		AT_Commandf("+QHTTPCFG=\"auth\",\"%s:%s\"", gBG95.auth.user, gBG95.auth.password);
		r = AT_ExpectOk();
		break;

	case BG95_Step_SetHTTPCFG_ContentType:
		AT_Command("+QHTTPCFG=\"contenttype\",2"); // application/octet-steam
		r = AT_ExpectOk();
		break;

	case BG95_Step_SetHTTP:
		if (gBG95.http.method[0] == 'G')
		{
			// Get request. No data.
			AT_Commandf("+QHTTP%s=%u",  gBG95.http.method, BG95_HTTP_TIMEOUT_S);
			r = AT_ExpectOk();
			if (r == AT_Ok)
				return BG95_Step_SetHTTP_Wait;
		}
		else
		{
			// PUT or POST request.
			// Data is expected.
			AT_Commandf("+QHTTP%s=%u,%u,%u", gBG95.http.method, (int)gBG95.http.tx_size, 5, BG95_HTTP_TIMEOUT_S);

			// Connection is established before the CONNECT comes though. WE need to wait for it.
			AT_SetTimeout((BG95_HTTP_TIMEOUT_S + 1) * 1000);
			r = AT_ExpectMatch("CONNECT");
		}
		break;

	case BG95_Step_SetHTTP_Content:
		AT_CommandRaw((uint8_t*)gBG95.http.tx_bfr, gBG95.http.tx_size);
		r = AT_ExpectOk();
		break;

	case BG95_Step_SetHTTP_Wait:
		AT_SetTimeout((BG95_HTTP_TIMEOUT_S + 1) * 1000);
		int response_code = 0, response_status = 0, response_size = 0;
		// This matches +QHTTP<method>:
		r = AT_ExpectMatchf(1, "+QHTTP%*[^:]: %u,%u,%u",
			&response_code, &response_status, &response_size
		);
		if (r == AT_Ok)
		{
			gBG95.http.status = response_status; // These parameters may not be recieved.
			gBG95.http.response_size = response_size;

			if (response_code != 0)
			{
				// This could be an HTTP failure? Not necessarily a connection issue.
				Log_Error("http failure");
				return BG95_Step_SendCallback;
			}

			Log_Info("%s %u (%u bytes)", gBG95.http.method, gBG95.http.status, gBG95.http.response_size);

			if (gBG95.http.rx_max == 0 || gBG95.http.response_size == 0)
				return BG95_Step_SendCallback; // No data

			if (gBG95.http.rx_max < gBG95.http.response_size)
			{
				// We cant read this.
				Log_Error("rx buffer (%d) too small for payload (%d)", gBG95.http.rx_max, gBG95.http.response_size);
				gBG95.http.response_size = 0;
				return BG95_Step_SendCallback;
			}

			// Lets read the request.
			return BG95_Step_HTTPRead;
		}
		break;

	case BG95_Step_HTTPRead:
		AT_Command("+QHTTPREAD");
		r = AT_ExpectMatch("CONNECT");
		break;

	case BG95_Step_HTTPRead_Content:
		// WARN: If more content is returned than can be parsed, this will gum up later requests....
		AT_SetTimeout(5000);
		r = AT_ExpectRaw(gBG95.http.rx_bfr, gBG95.http.response_size);
		break;

	case BG95_Step_HTTPRead_Ok:
		r = AT_ExpectOk();
		break;

	case BG95_Step_HTTPRead_Status:
		int http_read_status;
		r = AT_ExpectMatchf(1, "+QHTTPREAD: %d", &http_read_status);
		if (r == AT_Ok && http_read_status != 0)
		{
			Log_Error("Error during http read");
			return BG95_Step_Error;
		}
		break;

	case BG95_Step_SendCallback:
		TASK_CLEAR(BG95_Task_HTTP);
		if (gBG95.http.callback)
		{
			gBG95.http.callback(gBG95.http.status, gBG95.http.response_size);

			if (gBG95.step != BG95_Step_SendCallback)
			{
				// The user issued a state change within the callback.
				// Do not intefere with the new state change.
				return gBG95.step;
			}
		}
		return BG95_Step_Idle;
	}

	// Default cases top simplify unhandled cases above.
	switch (r)
	{
	case AT_Pending:	return step;
	case AT_Ok:			return ++step;
	default:			return BG95_Step_Error;
	}
}


#ifdef BG95_USE_PSM
static int BG95_ComputePsmTimer(uint32_t seconds, bool tau_format)
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
#endif //BG95_USE_PSM



#include "FPROM.h"
#include "FLASH.h"


/*
 * PRIVATE DEFINITIONS
 */

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

/*
 * PRIVATE VARIABLES
 */

// The size of the 'page' that we read and write from.
#ifndef FPROM_PAGE_SIZE
#define FPROM_PAGE_SIZE		256
#endif

/*
 * PUBLIC FUNCTIONS
 */

void FPROM_Read(uint32_t offset, void * data, uint32_t size)
{
	uint32_t page_n = FLASH_GetPageCount() - 1;
	const uint32_t * page = FLASH_GetPage(page_n);
	memcpy(data, (void*)page + offset, size);
}

void FPROM_Write(uint32_t offset, const void * data, uint32_t size)
{
	uint32_t page_n = FLASH_GetPageCount() - 1;
	const uint32_t * page = FLASH_GetPage(page_n);

	// Copy existing flash page
	uint8_t bfr[FPROM_PAGE_SIZE];
	memcpy(bfr, page, sizeof(bfr));

	// Modify flash page
	memcpy(bfr + offset, data, size);

	// TODO: Skip page erase if erase not required.

	// Write page
	FLASH_Erase(page);
	FLASH_Write(page, (const uint32_t*)bfr, sizeof(bfr));
}

/*
 * PRIVATE FUNCTIONS
 */

/*
 * INTERRUPT ROUTINES
 */


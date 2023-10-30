
#include "W25X20.h"
#include "SPI.h"
#include "GPIO.h"
#include "CORE.h"

/*
 * PRIVATE DEFINITIONS
 */


#define W25X20_CMD_WRITE_EN				0x06
#define W25X20_CMD_WRITE_EN_SR			0x50
#define W25X20_CMD_WRITE_DIS			0x04
#define W25X20_CMD_READ_SR				0x05
#define W25X20_CMD_WRITE_SR				0x01
#define W25X20_CMD_READ_DATA			0x03
#define W25X20_CMD_READ_DATA_F			0x0B
#define W25X20_CMD_READ_DATA_FDO		0x3B
#define W25X20_CMD_READ_DATA_FDIO		0xBB
#define W25X20_CMD_WRITE_DATA			0x02
#define W25X20_CMD_ERASE_4K				0x20
#define W25X20_CMD_ERASE_32K			0x52
#define W25X20_CMD_ERASE_64K			0xD8
#define W25X20_CMD_ERASE_CHIP			0xC7
#define W25X20_CMD_PDOWN				0xB9
#define W25X20_CMD_RELEASE_PDOWN		0xAB
#define W25X20_CMD_READ_DEVID			0x90
#define W25X20_CMD_READ_DEVID_DIO		0x92
#define W25X20_CMD_READ_JEDEC			0x9F
#define W25X20_CMD_READ_UID				0x4B


#define W25X20_SR_BUSY					0x01
#define W25X20_SR_WEL					0x02
#define W25X20_SR_BP0					0x04
#define W25X20_SR_BP1					0x08
#define W25X20_SR_TB					0x20
#define W25X20_SR_SRP					0x80


#define SIZE_64K 	(64 * 1024)
#define SIZE_32K 	(32 * 1024)
#define SIZE_4K 	(4 * 1024)

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */


static inline void W25X20_Select(void);
static inline void W25X20_Deselect(void);
static void W25X20_Command(uint8_t cmd);
static uint8_t W25X20_ReadSR(void);
static void W25X20_WaitForBusy(uint32_t timeout);
static void W25X20_EraseCommand(uint8_t erase_cmd, uint32_t addr);

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

void W25X20_Init(void)
{
	GPIO_EnableOutput(W25X20_CS_PIN, GPIO_PIN_SET);
}

void W25X20_Deinit(void)
{
	GPIO_Deinit(W25X20_CS_PIN);
}

void W25X20_Read(uint32_t addr, uint8_t * data, uint32_t size)
{
	W25X20_Select();
	uint8_t cmd[] = {
		W25X20_CMD_READ_DATA,
		addr >> 16,
		addr >> 8,
		addr,
	};
	SPI_Write(W25X20_SPI, cmd, sizeof(cmd));
	SPI_Read(W25X20_SPI, data, size);
	W25X20_Deselect();
}

void W25X20_Write(uint32_t addr, const uint8_t * data, uint32_t size)
{
	W25X20_Command(W25X20_CMD_WRITE_EN);

	W25X20_Select();
	uint8_t cmd[] = {
		W25X20_CMD_WRITE_DATA,
		addr >> 16,
		addr >> 8,
		addr,
	};
	SPI_Write(W25X20_SPI, cmd, sizeof(cmd));
	SPI_Write(W25X20_SPI, data, size);
	W25X20_Deselect();

	W25X20_WaitForBusy(300);
}

void W25X20_GetUID(uint8_t * uid)
{
	W25X20_Select();
	uint8_t cmd[] = {
		W25X20_CMD_READ_UID,
		0, 0, 0, 0,
	};
	SPI_Write(W25X20_SPI, cmd, sizeof(cmd));
	SPI_Read(W25X20_SPI, uid, 8);
	W25X20_Deselect();
}

void W25X20_Erase(uint32_t pos, uint32_t size)
{
	while (size) {
		// Try our best to use aligned commands.
		if (((pos & (SIZE_64K - 1)) == 0) && size >= SIZE_64K)
		{
			W25X20_EraseCommand(W25X20_CMD_ERASE_64K, pos);
			pos += SIZE_64K;
			size -= SIZE_64K;
		}
		else if (((pos & (SIZE_32K - 1)) == 0) && size >= SIZE_32K)
		{
			W25X20_EraseCommand(W25X20_CMD_ERASE_32K, pos);
			pos += SIZE_32K;
			size -= SIZE_32K;
		}
		else
		{
			W25X20_EraseCommand(W25X20_CMD_ERASE_4K, pos);
			pos += SIZE_4K;
			if (size <= SIZE_4K)
			{
				return;
			}
			size -= SIZE_4K;
		}
	}
}

void W25X20_EraseChip(void)
{
	W25X20_Command(W25X20_CMD_WRITE_EN);
	W25X20_Command(W25X20_CMD_ERASE_CHIP);
	W25X20_WaitForBusy(2000);
}

uint32_t W25X20_ReadSize(void)
{
	W25X20_Select();
	SPI_TransferByte(W25X20_SPI, W25X20_CMD_READ_JEDEC);
	uint8_t jedec[3];
	SPI_Read(W25X20_SPI, jedec, sizeof(jedec));
	W25X20_Deselect();

	return 1 << jedec[2];
}

/*
 * PRIVATE FUNCTIONS
 */

static inline void W25X20_Select(void)
{
	GPIO_Reset(W25X20_CS_PIN);
}

static inline void W25X20_Deselect(void)
{
	GPIO_Set(W25X20_CS_PIN);
}

static void W25X20_Command(uint8_t cmd)
{
	W25X20_Select();
	SPI_TransferByte(W25X20_SPI, cmd);
	W25X20_Deselect();
}

static uint8_t W25X20_ReadSR(void)
{
	W25X20_Select();
	SPI_TransferByte(W25X20_SPI, W25X20_CMD_READ_SR);
	uint8_t sr = SPI_TransferByte(W25X20_SPI, 0xFF);
	W25X20_Deselect();
	return sr;
}

static void W25X20_WaitForBusy(uint32_t timeout)
{
	uint32_t start = CORE_GetTick();
	while (CORE_GetTick() - start < timeout)
	{
		if (!(W25X20_ReadSR() & W25X20_SR_BUSY))
		{
			return;
		}
	}
}

static void W25X20_EraseCommand(uint8_t erase_cmd, uint32_t addr)
{
	W25X20_Command(W25X20_CMD_WRITE_EN);

	W25X20_Select();
	uint8_t cmd[] = {
		erase_cmd,
		addr >> 16,
		addr >> 8,
		addr,
	};
	SPI_Write(W25X20_SPI, cmd, sizeof(cmd));
	W25X20_Deselect();

	W25X20_WaitForBusy(1000);
}

/*
 * INTERRUPT ROUTINES
 */


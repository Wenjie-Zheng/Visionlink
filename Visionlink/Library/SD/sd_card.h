#ifndef __SD_CARD_H_
#define __SD_CARD_H_

#ifdef __cplusplus
extern "C"{
#endif
	
#include "my_drive.h"
#include "diskio.h"
#include "ff.h"
	
#define SD_CS_PORT		{GPIOB, GPIO_Pin_12}
	
typedef enum
{
	GO_IDLE_STATE          = 0,		//卡复位
	SEND_OP_COND           = 1,
	SEND_CID               = 2,
	SEND_RELATIVE_ADDR     = 3,
	SEND_SWITCH_FUNC       = 6,
	SEND_IF_COND           = 8,		//命令8 ，SEND_IF_COND
	SEND_CSD               = 9,		//命令9 ，读CSD数据
	STOP_TRANSMISSION      = 12,	//命令12，停止数据传输
	SEND_STATUS            = 13,
	SET_BLOCKLEN           = 16,	//命令16，设置SectorSize 应返回0x00
	READ_BLOCK_SINGLE      = 17,	//命令17，读sector
	READ_BLOCK_MULTIPLE    = 18,	//命令18，读Multi sector
	SEND_NUM_WR_BLOCKS     = 22,
	SET_WR_BLK_ERASE_COUNT = 23,	//命令23，设置多sector写入前预先擦除N个block
	WRITE_BLOCK_SINGLE     = 24,	//命令24，写sector
	WRITE_BLOCK_MULTIPLE   = 25,	//命令25，写Multi sector
	APP_OP_COND            = 41,	//命令41，应返回0x00
	APP_CLR_CARD_DETECT    = 42,	
	APP_CMD                = 55,	//命令55，应返回0x01
	READ_OCR               = 58,	//命令58，读OCR信息
	CRC_ON_OFF             = 59		//命令59，使能/禁止CRC，应返回0x00
}sdcard_command_t;	// SD卡指令表  
	
typedef enum
{
	 MSD_DATA_OK			= 0x05,
	 MSD_DATA_CRC_ERROR		= 0x0B,
	 MSD_DATA_WRITE_ERROR	= 0x0D,
	 MSD_DATA_OTHER_ERROR	= 0xFF
}sdcard_back_t;	//数据写入回应字意义
	
typedef enum {
	CARD_NONE	= 0x00,
	CARD_MMC	= 0x01,
	CARD_SDV1	= 0x02,
	CARD_SDV2	= 0X04,
	CARD_SDV2HC	= 0x06,
	CARD_UNKNOWN
}sdcard_type_t;	//sd卡类型
	
class SD_CARD : public SPI_lib
{
public:
	SD_CARD(SPI_TypeDef* SPIx, gpio_port_t cs_port)
		: SPI_lib(SPIx, cs_port)
	{
		sdType = CARD_UNKNOWN;
		sdSectors = 0;
		supports_crc = false;
		sdStatus = STA_NOINIT; //默认初始化失败
	}
	
	uint8_t begin(void);
	sdcard_type_t getCardType(void);
	uint64_t getCardSize(void);
	
	bool sdWait(uint16_t timeOut, uint8_t pdrv = 0);
	bool sdSelectCard(uint8_t pdrv = 0);
	void sdDeselectCard(uint8_t pdrv = 0);
	uint8_t sdTransaction(uint8_t cmd, uint32_t arg, uint32_t* resp);
	uint8_t sdCommand(uint8_t cmd, uint32_t arg);
	
	bool sdReadData(uint8_t *buffer, uint32_t len); //1: read succee 0: read fail
	
	uint8_t sdSetSectorsSize(uint16_t size);
	uint32_t sdGetSectorsCount(void);
	uint8_t sdGetCSD(uint8_t *csd);
	
	bool sdReadSectors(uint8_t* buffer, uint32_t sector, uint32_t count),
		 sdReadSector(uint8_t* buffer, uint32_t sector),
		 sdWriteSectors(uint8_t* buffer, uint32_t sector, uint32_t count),
		 sdWriteSector(uint8_t* buffer, uint32_t sector);
	
	uint8_t sdWriteBlock(uint8_t* buffer, uint8_t cmd);
	
	DRESULT sdReadDisk(uint8_t *buffer, uint32_t sector, uint32_t cout),
			sdWriteDisk(uint8_t *buffer, uint32_t sector, uint32_t cout);
	
	
	
	sdcard_type_t sdType;
	uint32_t sdSectors;
	
protected:
	uint32_t sdStatus;
	bool supports_crc;
};
	
	
	
	
	
	
	
	
	
	
	
#ifdef __cplusplus	
}
#endif	/* #ifdef __cplusplus */

#endif /* #ifndef __SD_CARD_H_ */
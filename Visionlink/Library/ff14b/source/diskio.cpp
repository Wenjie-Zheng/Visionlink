/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "sd_card.h"
//#include "sd_card.h"

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */

#define CARD_SD		0

extern SD_CARD sd;
/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = 0;

	switch (pdrv) {
	case CARD_SD :
		//result = RAM_disk_status();
		// translate the reslut code here
		return stat;
		
	default:
		break;
	}
	return STA_NOINIT;
}

	

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = STA_NOINIT;

	switch (pdrv) {

	case CARD_SD :
		stat = sd.begin();
		if (stat)	//STM32 SPI的bug,在sd卡操作失败的时候如果不执行下面的语句,可能导致SPI读写异常
		{
			sd.setSpeedLow();
			sd.readWriteByte(0xff); //提供额外的8个时钟
			sd.setSpeedHigh();
		}
		return stat;
	
	default:
		break;
	}
	
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	if (!count) return RES_PARERR;	//count不能等于0，否则返回参数错误		

	switch (pdrv) {

	case CARD_SD :
		res = sd.sdReadDisk(buff, sector, count);
		if (res)//STM32 SPI的bug,在sd卡操作失败的时候如果不执行下面的语句,可能导致SPI读写异常
		{
			sd.setSpeedLow();
			sd.readWriteByte(0xff); //提供额外的8个时钟
			sd.setSpeedHigh();
		}
		return res;
		
	default:
		break;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;

	if (!count) return RES_PARERR;	//count不能等于0，否则返回参数错误		

	switch (pdrv) {
		
	case CARD_SD :
		res = sd.sdWriteDisk((uint8_t *)buff, sector, count);
		return res;

	default:
		break;
	}

	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res = RES_PARERR;
	int result;

	switch (pdrv) {

	case CARD_SD :
		// Process of the command for the MMC/SD card
		switch (cmd)
		{
			case CTRL_SYNC:
				if (sd.sdSelectCard()) {
					res = RES_OK;
				}else {
					res = RES_ERROR;
				}
				sd.sdDeselectCard();
				break;
			
			case GET_SECTOR_COUNT:
				*(DWORD*)buff = sd.sdSectors;
				res = RES_OK;
				break;
			
			case GET_SECTOR_SIZE:
				*(WORD*)buff = 512;
				res = RES_OK;
				break;
			
			case GET_BLOCK_SIZE:
				*(WORD*)buff = 8;
				res = RES_OK;
				break;	 
			
			default:
				res = RES_PARERR;
				break;
		}
		return res;
		
	default:
		break;
	}

	return RES_PARERR;
}

DWORD get_fattime (void)
{				 
	return 0;
}		

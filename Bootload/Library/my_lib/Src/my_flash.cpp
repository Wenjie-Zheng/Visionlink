/**
	******************************************************************************
	*	@file    flash_lib.c
	* @author	 Monster
	* @version V1.0.0
	* @brief	 User flash operation library function.
	******************************************************************************
	*/
/* Includes ------------------------------------------------------------------*/
#include "my_flash.h"
#include <string.h>
#include <stm32f10x_flash.h>

/* Global variable -----------------------------------------------------------*/

/**
 * @brief  Get input address in flash what page.
 * @param  addr: intput address.
 * @retval flash page number.
 */
uint16_t mylib_flash::get_flash_page(uint32_t addr)
{
	uint32_t off_addr;
	uint16_t page;
	
	off_addr = addr - FLASH_BASE;
	page = off_addr / SECTOR_SIZE;
	
	return page;
}

/**
 * @brief  Erase the page used from the start address to the end address.
 * @param  start_addr: intput start address.
 * @param  end_addr：intput end address.
 * @retval success return 1 or fail return 0.
 */
uint8_t mylib_flash::erase_flash_page(uint32_t start_addr, uint32_t end_addr)
{
	uint16_t start_page_index = 0, end_page_index = 0;
	uint8_t i;
	
	if(start_addr < FLASH_BASE || start_addr > (FLASH_BASE + FLASH_SIZE*1024))
		return 0;
	
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	start_page_index = get_flash_page(start_addr);
	end_page_index   = get_flash_page(end_addr);
	
	for(i = start_page_index; i <= end_page_index; i++)
	{
		if( FLASH_ErasePage(i*SECTOR_SIZE + FLASH_BASE) != FLASH_COMPLETE )
			return 0;
	}
	FLASH_Lock();
	return 1;
}

/**
 * @brief  Writes data of the specified length from the starting address.
 * @param  start_addr: intput start address.
 * @param  write_buff: intput end address.
 * @param	 write_size: Write data length.
 * @retval success return 1 or fail return 0.
 */
uint8_t mylib_flash::program_flash(uint32_t start_addr, uint32_t* const write_buff, uint32_t write_size)
{
	uint32_t *pBuff = write_buff;
	uint32_t program_addr = start_addr;
	
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	
	while(program_addr < (start_addr + write_size) )
	{
		if( FLASH_ProgramWord(program_addr, *pBuff) == FLASH_COMPLETE )
		{
			program_addr += 4;
			pBuff ++;
		}
		else
			return 0;
	}
	FLASH_ClearFlag(FLASH_FLAG_EOP);
	FLASH_Lock();
	
	return 1;
}

/**
 * @brief  Read flash address data to buffer.
 * @param  start_addr: read start address.
 * @param	 read_buff: storage bubber pointer.
 * @param	 read_size: Read length.
 * @retval Success returns 1, failure returns 0.
 */
uint8_t mylib_flash::read_flash(uint32_t start_addr, uint32_t *read_buff, uint32_t read_size)
{
	uint32_t read_addr = start_addr;
	if(start_addr < FLASH_BASE || start_addr > (FLASH_BASE + FLASH_SIZE*1024))
		return 0;
	while(read_addr < (start_addr + read_size))
	{
		*read_buff++ = *(__IO uint32_t*)read_addr;
		read_addr += 4;
	}
	return 1;
}




#ifndef __MY_FLASH_H
#define __MY_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stm32f10x.h>

#define FLASH_SIZE	64		//所选STM32的FLASH容量大小(单位为K)

//定义扇区大小
#if FLASH_SIZE <= 256
	#define SECTOR_SIZE  1024
#else
	#define SECTOR_SIZE  2048
#endif

class mylib_flash
{
public:
	uint8_t erase_flash_page(uint32_t start_addr, uint32_t end_addr);
	uint8_t program_flash(uint32_t start_addr, uint32_t* const write_buff, uint32_t write_size);
	uint8_t read_flash(uint32_t start_addr, uint32_t *read_buff, uint32_t read_size);
private:
	uint16_t get_flash_page(uint32_t addr);
};


#ifdef __cplusplus
}	/*extern C*/
#endif
	
#endif /* __MY_FLASH_H_ */

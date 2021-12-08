#include "my_drive.h"
#include "tft_lcd.h"
#include "sd_card.h"
#include "string.h"
#include "dog.h"
#include <stdio.h>

USART_lib usart1(USART1, 115200, USART_Mode_Tx);
TFT_eSPI tft(240, 240);
SD_CARD sd(SPI2, SD_CS_PORT);

uint8_t image_buff[14400];

FRESULT scan_files(char* path); 
void sdread_image(uint8_t index);


FATFS fatfs;
FIL fil;
UINT bw;

char dir_buf[256];

char image_dir[32][32];
uint16_t image_num = 0;

int main()
{
	delay_init();
	Led_Init();
	pinMode(GPIOA, GPIO_Pin_0, GPIO_Mode_IPU);
	pinMode(GPIOA, GPIO_Pin_1, GPIO_Mode_IPU);
	//usart1.dma_init(USART_DMAReq_Rx, image_buff, sizeof(image_buff));
	//usart1.set_rx_inter_cb(USART_IT_IDLE, 1, user_cb);
	
	tft.init(SPI1);
	tft.setRotation(3);
	tft.fillScreen(TFT_BLACK);
	tft.setTextSize(2);

	tft.setSwapBytes(true);
	tft.pushImage(80, 70, 80, 97, gImage_dog);
	tft.drawString("Ready!", 90, 170);
	printf("Being initialized SD card\n");
	while (sd.begin())
	{
		printf(".\n");
		Led_toggle();
		delay_ms(500);
	}
	printf("\r\nSD card Initialize succeed!\r\n");

	f_mount(&fatfs, "0:/", 1);
	strcat(dir_buf, "0:/Image");
	FRESULT res = scan_files(dir_buf);
	
	uint8_t image_index = 0;
	while (1)
	{
		if (!pinRead(GPIOA, GPIO_Pin_1))
		{
			if (image_index < (image_num - 1))
				image_index++;
			
			printf("Current Displayable : %s\r\n", image_dir[image_index]);
			sdread_image(image_index);
		}
		
		if (!pinRead(GPIOA, GPIO_Pin_0))
		{
			if (image_index > 0)
				image_index--;
			
			sdread_image(image_index);
		}

	}
}

void sdread_image(uint8_t index)
{		
	FRESULT res;
	res = f_open(&fil, image_dir[index], FA_OPEN_EXISTING | FA_READ);
	if (res == FR_OK){
		for (uint8_t index = 0; index < 8; index++)
		{
			f_read(&fil, image_buff, 14400, &bw);
			tft.pushImage(0, index * 30, 240, 30, image_buff);
			f_lseek(&fil, (index + 1) * 14400);
		}
	}
	else{
		f_mount(&fatfs, "0:/", 0);
		delay_ms(100);
		res = f_mount(&fatfs, "0:/", 1);
		if (res != FR_OK)
		{
			printf("Read SD card fail!\r\n");
		}
	}
	f_close(&fil);
}


FRESULT scan_files(
    char* path        /* Start node to be scanned (***also used as work area***) */
)
{
	FRESULT res;
	DIR dir;
	UINT i;
	static FILINFO fno;

	res = f_opendir(&dir, path); /* Open the directory */
	if (res == FR_OK) {
		for (;;) {
			res = f_readdir(&dir, &fno); /* Read a directory item */
			if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
			if (fno.fattrib & AM_DIR) {
				/* It is a directory */
				i = strlen(path);
				sprintf(&path[i], "/%s", fno.fname);
				res = scan_files(path); /* Enter the directory */
				if (res != FR_OK) break;
				path[i] = 0;
			}
			else {
				/* It is a file. */
				printf("%s/%s\r\n", path, fno.fname);
				strcat(image_dir[image_num], path);
				strcat(image_dir[image_num], "/");
				strcat(image_dir[image_num], fno.fname);
				image_num++;
			}
		}
		f_closedir(&dir);
	}

	return res;
}


void indexed_color_16(uint16_t x, uint16_t y, uint16_t h, uint16_t w, const uint8_t *data, uint8_t *cmap)
{
	uint16_t hlen = h;
	uint16_t wlen = w/2;
	uint16_t linebuf[w];
	uint16_t index;
	uint16_t i = 0;
	
	tft.setWindow(x, y, x + w - 1, y + w - 1);
	
	while (hlen--)
	{
		
		while (wlen--)
		{
			uint8_t read_data =  pgm_read_byte(data++);
			
			index = (*data >> 4) | 0x0f;
			linebuf[i++] = RGB555(cmap[index * 4] , cmap[index * 4 + 1] , cmap[index * 4 + 2]);
			index = (*data) | 0x0f;
			linebuf[i++] = RGB555(cmap[index * 4], cmap[index * 4 + 1], cmap[index * 4 + 2]);
			;
		}
		i = 0;
		wlen = w / 2;
		tft.pushPixels(linebuf, w);
	}
}
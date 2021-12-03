#include "my_drive.h"
#include "tft_lcd.h"
#include "sd_card.h"
#include "string.h"
#include "dog.h"


TFT_eSPI tft(240, 240);
SD_CARD sd(SPI2, SD_CS_PORT);
USART_lib usart1(USART1, 115200, USART_Mode_Tx);
USART_lib usart2(USART1, 115200, USART_Mode_Tx | USART_Mode_Rx);

uint8_t cat_buff[14400];

char cat_buf[8][21] = {
	{ "0:Image/cat/cat1.bin" },{ "0:Image/cat/cat2.bin" },
	{ "0:Image/cat/cat3.bin" },{ "0:Image/cat/cat4.bin" },
	{ "0:Image/cat/cat5.bin" },{ "0:Image/cat/cat6.bin" },
	{ "0:Image/cat/cat7.bin" },{ "0:Image/cat/cat8.bin" }
};

char sao_cat[8][18] = {
	{ "0:Image/sao/1.bin" },{ "0:Image/sao/2.bin" },
	{ "0:Image/sao/3.bin" },{ "0:Image/sao/4.bin" },
	{ "0:Image/sao/5.bin" },{ "0:Image/sao/6.bin" },
	{ "0:Image/sao/7.bin" },{ "0:Image/sao/8.bin" },
};


FATFS fatfs;
FIL fil;
UINT bw;
int main()
{
	delay_init();
	Led_Init();
	pinMode(GPIOA, GPIO_Pin_8, GPIO_Mode_IN_FLOATING);
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
	while (sd.begin())
	{
		Led_toggle();
		delay_ms(500);
	}
	
	f_mount(&fatfs, "0:", 1);
	
	uint8_t data_len = 0;
	while (1)
	{
		if (!pinRead(GPIOA, GPIO_Pin_1))
		{
			do
			{
				f_open(&fil, sao_cat[data_len], FA_READ);
				f_read(&fil, cat_buff, 14400, &bw);
				tft.pushImage(0, data_len * 30, 240, 30, cat_buff);
				f_close(&fil);
			} while (++data_len < 8);
			data_len = 0;
		}
		
		if (!pinRead(GPIOA, GPIO_Pin_0))
		{
			do
			{
				f_open(&fil, cat_buf[data_len], FA_READ);
				f_read(&fil, cat_buff, 14400, &bw);
				tft.pushImage(0, data_len * 30, 240, 30, cat_buff);
				f_close(&fil);
			} while (++data_len < 8);
			data_len = 0;
		}
	}
	
}

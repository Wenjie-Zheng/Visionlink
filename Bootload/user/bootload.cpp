#include "esp_cmd.h"
#include "my_drive.h"
#include "stm32f10x_usart.h"
#include <string.h>
//ESP_MOD esp8266(USART1, 115200, USART_Mode_Tx | USART_Mode_Rx, 3, 500);


USART_lib usart1(USART1, 115200, USART_Mode_Tx | USART_Mode_Rx);
void user_cb(buff_t *buf)
{
	usart1.send_data(buf->buf, buf->data_size);
}

uint8_t usart_rx_buf[64];
int main(void)
{
	delay_init();
	Led_Init();
	
	usart1.dma_init(USART_DMAReq_Rx, usart_rx_buf, 64);
	usart1.set_rx_inter_cb(USART_IT_IDLE, 1, user_cb);
	while(1)
	{
		
		//esp8266.usart_send_byte(0xA5);
//		esp8266.send_cmd((uint8_t *)ESPAT_CREATE, (uint8_t *)"OK");
		delay_ms(500);
	}
}
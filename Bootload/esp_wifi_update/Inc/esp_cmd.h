#ifndef __ESP_CMD_H
#define __ESP_CMD_H

#ifdef __cplusplus
extern "C"{
#endif
#include <stdint.h>
#include "my_drive.h"

	
#define RX_BUFF_SIZE	(1024 +25 +6 +8)
	
#define ESPAT_RST		"AT+RST\r\n"
#define ESPAT_CHECK		"AT\r\n"
#define ESPAT_MODE_AP	"AT+CWMODE=2\r\n"
#define ESPAT_CREATE	"AT+CWSAP=\"Monster\",\"123456789\",1,4\r\n"
#define ESPAT_MUX		"AT+CIPMUX=1\r\n"
#define ESPAT_SERVER	"AT+CIPSERVER=1,80\r\n"

class ESP_MOD :public USART_lib
{
protected:

public:
	ESP_MOD(USART_TypeDef* USARTx, uint32_t baud, uint16_t usart_mode, uint8_t retry_number, uint16_t ack_time)
		: USART_lib(USARTx, baud, usart_mode)
	{
		this->retry_number = retry_number;
		this->ack_time = ack_time;
	}
	void send_cmd(uint8_t *cmd, uint8_t *ack);
	
private:
	uint8_t retry_number = 1;
	uint16_t ack_time = 0;
	
};
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	#ifdef __cplusplus
	}
#endif // __cplusplus


#endif // __ESP_CMD_H

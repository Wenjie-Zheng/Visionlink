#include "esp_cmd.h"
#include <string.h>


void ESP_MOD::send_cmd(uint8_t *cmd, uint8_t *ack)
{
	uint8_t try_index = this->retry_number;
//	do
//	{
		//memset(&(this->rx_buf), 0, sizeof(rx_buf));
		this->send_data((const uint8_t *)cmd, strlen((char*)cmd));
//	} while (try_number-- > 0);
}

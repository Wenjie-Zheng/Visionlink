#ifndef __MY_DRIVE_H
#define	__MY_DRIVE_H

#ifdef __cplusplus
extern "C"{
#endif
	
/* Private include -----------------------------------------------------------*/
#include "stm32f10x.h"
#include <stm32f10x_gpio.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_dma.h>
#include <stm32f10x_spi.h>
	
/* Private typedef -----------------------------------------------------------*/
	
typedef struct
{
	GPIO_TypeDef*	GPIO;
	uint16_t		Pin;
}gpio_port_t;
	
typedef struct
{
	uint8_t  *buf;
	uint16_t data_size;
	uint16_t buf_size;
}buff_t;
	
typedef void(*IRQ_Callback)(buff_t* buf);
	
typedef struct
{
	IRQ_Callback callback;
	buff_t* data;
	uint16_t it_type;
	DMA_Channel_TypeDef* dma;
}usart_it_t;
	
	

/* Private define ------------------------------------------------------------*/

/* Private extern ------------------------------------------------------------*/
extern usart_it_t usart1_it;
extern usart_it_t usart2_it;
extern usart_it_t usart3_it;
/* Private functions ---------------------------------------------------------*/
void delay_init(void);
void delay_us(uint16_t xus);
void delay_ms(uint16_t xms);
void HSI_ClockSet(void);
void pinMode(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIOMode_TypeDef Mode);
void pinSet(GPIO_TypeDef* GPIOx, uint16_t GPIO_pin, uint8_t BitVal);
uint8_t pinRead(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
	
void Led_Init(void);
void Led_toggle(void);
	
	
/* Class functions -----------------------------------------------------------*/
void usart1_recive_callback(void);
void usart2_recive_callback(void);
void usart3_recive_callback(void);
	
	
class USART_lib
{
protected:
	buff_t tx;
	buff_t rx;

	void usart_nvic_init(uint8_t Priority);
	void usart_tx_dma_init(uint8_t* tx_buff);
	void usart_rx_dma_init(uint8_t* rx_buff);
public:
	USART_lib(USART_TypeDef* USARTx, uint32_t baud, uint16_t usart_mode);
	void dma_init(uint16_t USART_DMAReq, uint8_t *buff, uint16_t buf_size);
	void set_rx_inter_cb(uint16_t USART_IT, uint8_t Priority, IRQ_Callback callback);
	void send_byte(uint8_t data);
	void send_data(const uint8_t* send_buff, uint16_t size);
	
private:
	USART_TypeDef* USART;
	uint16_t USART_IT;
	DMA_Channel_TypeDef* TX_Channel;
	DMA_Channel_TypeDef* RX_Channel;
	uint8_t NVIC_IRQ;	
};
	
class SPI_lib
{
public:
	SPI_lib(SPI_TypeDef* SPIx, gpio_port_t cs_port);
	void	init();
	void	setSelection(uint8_t BitVal);
	
	void	writeByte(uint8_t data),
			writeByte(uint16_t data),
			writeByte(uint32_t data),
			writeData(const uint8_t* data_in, uint32_t len);
	uint8_t readByte(void),
			readWriteByte(uint8_t data);
	uint16_t readWriteHalfWord(uint16_t data);
	uint32_t readWriteWord(uint32_t data);
	void	setSpeedLow(void),
			setSpeedHigh(void);
	
	gpio_port_t	 CS;
private:
	SPI_InitTypeDef spi_config;
	SPI_TypeDef* SPI;
};
	
	

	
#ifdef __cplusplus
}	/* extern "C" */
#endif

#endif /* __MY_LIB_H */



/*include file --------------------------------------*/
#include "my_drive.h"
#include <misc.h>
#include <stm32f10x_rcc.h>
#include <stdio.h>

/* Private variables ---------------------------------------------------------*/
static uint8_t fac_us;
static uint16_t fac_ms;
usart_it_t usart1_it;
usart_it_t usart2_it;
usart_it_t usart3_it;

/* Private functions ---------------------------------------------------------*/
#ifdef __cplusplus
int fputc(int ch, FILE* stream)
{
	while ((USART1->SR & USART_FLAG_TC) == RESET) {}
	; 
	USART1->DR = (uint8_t) ch;      
	return ch;
}
#else
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while ((USART1->SR & USART_FLAG_TC) == RESET) {}
	;//循环发送,直到发送完毕   
	USART1->DR = (uint8_t) ch;      
	return ch;
}
#endif


void delay_init(void)
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8); //选择外部时钟  HCLK/8
	fac_us = SystemCoreClock / 8000000; //为系统时钟的1/8  
	fac_ms = (uint16_t)fac_us * 1000;	
}
						    						   
void delay_us(uint16_t xus)
{
	uint32_t temp;	    	 
	SysTick->LOAD = xus*fac_us; //时间加载	  		 
	SysTick->VAL = 0x00; //清空计数器
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; //开始倒数	  
	do
	{
		temp = SysTick->CTRL;
	} while ((temp & 0x01) && !(temp&(1 << 16)));//等待时间到达   
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; //关闭计数器
	SysTick->VAL = 0X00; //清空计数器	 
}

void delay_ms(uint16_t xms)
{	 		  	  
	uint32_t temp;		   
	SysTick->LOAD = (uint32_t)xms*fac_ms; //时间加载(SysTick->LOAD为24bit)
	SysTick->VAL = 0x00; //清空计数器
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; //开始倒数  
	do
	{
		temp = SysTick->CTRL;
	} while ((temp & 0x01) && !(temp&(1 << 16)));//等待时间到达   
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; //关闭计数器
	SysTick->VAL = 0X00; //清空计数器	  	    
}


/**
 * @brief  This function Select HSI as the System clock.
 * @param  None
 * @retval None
 */
void HSI_ClockSet(void)
{
	//配置内置振荡器 36MHz
	RCC_DeInit(); /*将外设RCC寄存器重设为缺省值 */ 
	RCC_HSICmd(ENABLE); //使能内部时钟  HSI
	while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET) ;//等待HSI就绪 
	 
	RCC_HCLKConfig(RCC_SYSCLK_Div1); /*设置AHB时钟（HCLK） RCC_SYSCLK_Div1——AHB时钟 = 系统时*/  
	RCC_PCLK2Config(RCC_HCLK_Div1); /* 设置高速AHB时钟（PCLK2)RCC_HCLK_Div1——APB2时钟 = HCLK*/     
	RCC_PCLK1Config(RCC_HCLK_Div2); /*设置低速AHB时钟（PCLK1）RCC_HCLK_Div2——APB1时钟 = HCLK / 2*/   
 
	 //下面这语句设置时钟频率  记住参考
	RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_9); /*设置PLL时钟源及倍频系数，频率为8/2*9=36Mhz*/    
	RCC_PLLCmd(ENABLE); /*使能PLL *///这里使用的内部时钟所以不能被 失能 
	while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) ; /*检查指定的RCC标志位(PLL准备好标志)设置与否   等待是否配置成功*/    
	
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); /*设置系统时钟（SYSCLK） */  
	//选择哪一个做为时钟   
	//  0x00：HSI 作为系统时钟     	RCC_SYSCLKSource_HSI
	//  0x04：HSE作为系统时钟 			RCC_SYSCLKSource_HSE
	//  0x08：PLL作为系统时钟 			RCC_SYSCLKSource_PLLCLK 
	 
	while (RCC_GetSYSCLKSource() != 0x08) ;//需与被选择的系统时钟对应起来，RCC_SYSCLKSource_PLL
}

/**
 * @brief  This function Initialize GPIO.
 * @param  GPIOx, GPIO_Pin, Mode
 * @retval None
 */
void pinMode(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIOMode_TypeDef Mode)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	if (GPIOx == GPIOA)
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	else if (GPIOx == GPIOB)
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	else if (GPIOx == GPIOC)
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	else if (GPIOx == GPIOD)
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	else if (GPIOx == GPIOE)
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin;
	GPIO_InitStructure.GPIO_Mode	= Mode;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;

	GPIO_Init(GPIOx, &GPIO_InitStructure);
}

void pinSet(GPIO_TypeDef* GPIOx, uint16_t GPIO_pin, uint8_t BitVal)
{
	if (BitVal != Bit_RESET)
		GPIOx->ODR |= GPIO_pin;
	else
		GPIOx->ODR &= ~GPIO_pin;
}

uint8_t pinRead(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	if ((GPIOx->IDR & GPIO_Pin) != (uint32_t)Bit_RESET){
		return Bit_SET;
	}
	else{
		return Bit_RESET;
	}
}
/**
 * @brief  This function Initialize LED.
 * @param  GPIOx, GPIO_Pin, Mode
 * @retval None
 */
void Led_Init(void)
{
	pinMode(GPIOB, GPIO_Pin_6, GPIO_Mode_Out_PP);
	pinMode(GPIOB, GPIO_Pin_7, GPIO_Mode_Out_PP);
	pinMode(GPIOB, GPIO_Pin_8, GPIO_Mode_Out_PP);
	//GPIOB->ODR &= ~GPIO_Pin_6;
	GPIOB->ODR &= ~GPIO_Pin_7;
	GPIOB->ODR &= ~GPIO_Pin_8;
}
void Led_toggle(void)
{
	static uint8_t index = 0;
	index ^= 0x01;
	if (index) {
		//GPIOB->ODR &= ~GPIO_Pin_6;
		GPIOB->ODR &= ~GPIO_Pin_7;
		GPIOB->ODR &= ~GPIO_Pin_8;
	}
	else {
		//GPIOB->ODR |= GPIO_Pin_6;
		GPIOB->ODR |= GPIO_Pin_7;
		GPIOB->ODR = GPIO_Pin_8;
	}
}

/**
 * @brief  User serial port interrupt callback function.
 * @param  None.
 * @retval None
 */
void usart1_recive_callback(void)
{	
	uint16_t count = 0;
	if (USART_GetITStatus(USART1, usart1_it.it_type) != RESET)
	{
		USART_ClearITPendingBit(USART1, usart1_it.it_type);
		usart1_it.dma->CCR &= (uint16_t)(~DMA_CCR1_EN); //关闭串口DMA接收通道
		count = USART1->DR;
		count = USART1->SR;
		
		
		count = (usart1_it.data->buf_size) - ((uint16_t)(usart1_it.dma->CNDTR));
		if (count > 0)
		{
			usart1_it.data->data_size = count;
			usart1_it.callback(usart1_it.data);
		}
		//usart1_it.dma->CCR &= (uint16_t)(~DMA_CCR1_EN); //关闭串口DMA接收通道
		usart1_it.dma->CNDTR = usart1_it.data->buf_size; //指定DMA缓存区长度
		usart1_it.dma->CCR |= DMA_CCR1_EN; //开启DMA
	}
}

void usart2_recive_callback(void)
{
	uint16_t count = 0;
	if (USART_GetITStatus(USART1, usart2_it.it_type) != RESET)
	{
		USART_ClearITPendingBit(USART1, usart2_it.it_type);
		count = USART1->DR;
		count = USART1->SR;
		usart2_it.dma->CCR &= (uint16_t)(~DMA_CCR1_EN); //关闭串口DMA接收通道
		
		count = (usart2_it.data->buf_size) - (usart2_it.dma->CNDTR);
		if (count > 0)
		{
			usart2_it.data->data_size = count;
			usart2_it.callback(usart2_it.data);
		}
		usart2_it.dma->CCR &= (uint16_t)(~DMA_CCR1_EN); //关闭串口DMA接收通道
		usart2_it.dma->CNDTR = usart2_it.data->buf_size; //指定DMA缓存区长度
		usart2_it.dma->CCR |= DMA_CCR1_EN; //开启DMA
	}
}

void usart3_recive_callback(void)
{
	uint16_t count = 0;
	if (USART_GetITStatus(USART1, usart3_it.it_type) != RESET)
	{
		USART_ClearITPendingBit(USART1, usart3_it.it_type);
		count = USART1->DR;
		count = USART1->SR;
		usart3_it.dma->CCR &= (uint16_t)(~DMA_CCR1_EN); //关闭串口DMA接收通道
		
		count = (usart3_it.data->buf_size) - (usart3_it.dma->CNDTR);
		if (count > 0)
		{
			usart3_it.data->data_size = count;
			usart3_it.callback(usart3_it.data);
		}
		usart3_it.dma->CCR &= (uint16_t)(~DMA_CCR1_EN); //关闭串口DMA接收通道
		usart3_it.dma->CNDTR = usart3_it.data->buf_size; //指定DMA缓存区长度
		usart3_it.dma->CCR |= DMA_CCR1_EN; //开启DMA
	}
}

/**
 * @brief  Serial port class function definition.
 * @param  None.
 * @retval None
 */
USART_lib::USART_lib(USART_TypeDef* USARTx, uint32_t baud, uint16_t usart_mode)
{
	USART_InitTypeDef USART_InitStructure;
	
	this->USART = USARTx;

	if (USARTx == USART1) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
		pinMode(GPIOA, GPIO_Pin_9, GPIO_Mode_AF_PP);
		pinMode(GPIOA, GPIO_Pin_10, GPIO_Mode_IN_FLOATING);
		this->NVIC_IRQ = USART1_IRQn;
	}
	else if (USARTx == USART2) {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
		pinMode(GPIOA, GPIO_Pin_2, GPIO_Mode_AF_PP);
		pinMode(GPIOA, GPIO_Pin_3, GPIO_Mode_IN_FLOATING);
		this->NVIC_IRQ = USART2_IRQn;
	}
	else if (USARTx == USART3) {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
		pinMode(GPIOB, GPIO_Pin_10, GPIO_Mode_AF_PP);
		pinMode(GPIOA, GPIO_Pin_11, GPIO_Mode_IN_FLOATING);
		this->NVIC_IRQ = USART3_IRQn;
	}
	
	USART_InitStructure.USART_BaudRate = baud;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = usart_mode;
	USART_Init(this->USART, &USART_InitStructure);
	USART_Cmd(this->USART, ENABLE);
}


void USART_lib::usart_nvic_init(uint8_t Priority)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	uint8_t pre = 0, sub = 0;
	pre = Priority / 3;
	sub = Priority % 3;

	NVIC_InitStructure.NVIC_IRQChannel = this->NVIC_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = pre;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = sub;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void USART_lib::usart_tx_dma_init(uint8_t* tx_buff)
{
	DMA_InitTypeDef DMA_InitStructure;
	
	if (USART == USART1) {
		this->TX_Channel = DMA1_Channel4;
	}
	else if (USART == USART2) {
		this->TX_Channel = DMA1_Channel7;
	}
	else if (USART == USART3) {
		this->TX_Channel = DMA1_Channel2;
	}

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(this->USART->DR);
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)tx_buff;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = this->tx.buf_size;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(this->TX_Channel, &DMA_InitStructure);
	DMA_Cmd(this->TX_Channel, ENABLE);
}

void USART_lib::usart_rx_dma_init(uint8_t* rx_buff)
{
	DMA_InitTypeDef DMA_InitStructure;
	
	if (USART == USART1) {
		this->RX_Channel = DMA1_Channel5;
	}
	else if (USART == USART2) {
		this->RX_Channel = DMA1_Channel6;
	}
	else if (USART == USART3) {
		this->RX_Channel = DMA1_Channel3;
	}

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(this->USART->DR);
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)rx_buff;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = (uint32_t)(this->rx.buf_size);
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(this->RX_Channel, &DMA_InitStructure);
	DMA_Cmd(this->RX_Channel, ENABLE);
}


void USART_lib::dma_init(uint16_t USART_DMAReq, uint8_t *buff, uint16_t buf_size)
{
	USART_Cmd(this->USART, DISABLE);
	
	if (USART_DMAReq & USART_DMAReq_Tx)
	{
		this->tx.buf = buff;
		this->tx.buf_size = buf_size;
		this->usart_tx_dma_init(buff);
		USART_DMACmd(this->USART, USART_DMAReq_Tx, ENABLE);
	}
	if (USART_DMAReq & USART_DMAReq_Rx)
	{
		this->rx.buf = buff;
		this->rx.buf_size = buf_size;
		this->usart_rx_dma_init(buff);
		USART_DMACmd(this->USART, USART_DMAReq_Rx, ENABLE);
	}
	USART_Cmd(this->USART, ENABLE);
}


void USART_lib::send_byte(uint8_t data)
{
	while ((this->USART->SR & USART_FLAG_TXE) == 0) {}
	;
	this->USART->DR = data;
}

void USART_lib::send_data(const uint8_t* send_buff, uint16_t size)
{
	const uint8_t *p = send_buff;
	while (size--)
	{
		while ((this->USART->SR & USART_FLAG_TXE) == 0) {}
		;
		this->USART->DR = *(p++);
	}
}

void USART_lib::set_rx_inter_cb(uint16_t USART_IT, uint8_t Priority, IRQ_Callback callback)
{	
	USART_Cmd(this->USART, DISABLE);
	
	if (this->USART == USART1) {
		usart1_it.callback = callback;
		usart1_it.data = &this->rx;
		usart1_it.it_type = USART_IT;
		usart1_it.dma = this->RX_Channel;
	}
	else if (this->USART == USART2) {
		usart2_it.callback = callback;
		usart2_it.data = &this->rx;
		usart2_it.it_type = USART_IT;
		usart2_it.dma = this->RX_Channel;
	}
	else if (this->USART == USART3) {
		usart3_it.callback = callback;
		usart3_it.data = &this->rx;
		usart3_it.it_type = USART_IT;
		usart3_it.dma = this->RX_Channel;
	}
	this->usart_nvic_init(Priority);
	USART_ClearITPendingBit(this->USART, USART_IT);
	USART_ITConfig(this->USART, USART_IT, ENABLE);	
	USART_Cmd(this->USART, ENABLE);
}


SPI_lib::SPI_lib(SPI_TypeDef* SPIx, gpio_port_t cs_port)
{
	this->SPI = SPIx;
	this->CS  = cs_port;
	
	pinMode(CS.GPIO, CS.Pin, GPIO_Mode_Out_PP);
	pinSet(CS.GPIO, CS.Pin, Bit_SET);
}

void SPI_lib::init(void)
{
	if (SPI == SPI1) {
		//this->SPI = SPI1;
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
		pinMode(GPIOA, GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7, GPIO_Mode_AF_PP);
		pinSet(GPIOA, GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7, Bit_SET);
	}
	else if (SPI == SPI2) {
		//this->SPI = SPI2;
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
		pinMode(GPIOB, GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15, GPIO_Mode_AF_PP);
		pinSet(GPIOB, GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15, Bit_SET);
	}
	
	spi_config.SPI_Direction	= SPI_Direction_2Lines_FullDuplex;
	spi_config.SPI_Mode			= SPI_Mode_Master;
	spi_config.SPI_DataSize		= SPI_DataSize_8b;
	spi_config.SPI_CPOL			= SPI_CPOL_High;
	spi_config.SPI_CPHA			= SPI_CPHA_2Edge;
	spi_config.SPI_NSS			= SPI_NSS_Soft;
	spi_config.SPI_FirstBit		= SPI_FirstBit_MSB;
	spi_config.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	spi_config.SPI_CRCPolynomial = 7;
	SPI_Init(SPI, &spi_config);
	SPI_Cmd(SPI, ENABLE);
	
	pinSet(CS.GPIO, CS.Pin, Bit_SET);
	readWriteByte(0xff);
}

void SPI_lib::setSelection(uint8_t BitVal)
{
	if (BitVal != Bit_RESET) {
		pinSet(CS.GPIO, CS.Pin, Bit_SET);
	}
	else {
		pinSet(CS.GPIO, CS.Pin, Bit_RESET);
	}
}


void SPI_lib::writeByte(uint8_t data)
{
	while (((this->SPI)->SR & SPI_I2S_FLAG_TXE) == RESET) ;	//wait data send end
//	*((__IO uint8_t *)&SPI->DR) = Data;
	(this->SPI)->DR = data;
	while (((this->SPI)->SR & (SPI_I2S_FLAG_TXE | SPI_I2S_FLAG_BSY)) != SPI_I2S_FLAG_TXE) ;		//wait spi free
}

void SPI_lib::writeByte(uint16_t data)
{
	this->writeByte((uint8_t)(data >> 8));
	this->writeByte((uint8_t)data);
}

void SPI_lib::writeByte(uint32_t data)
{
	this->writeByte((uint16_t)(data >> 16));
	this->writeByte((uint16_t)data);
}

void SPI_lib::writeData(const uint8_t* data_in, uint32_t len)
{
	while (len--) {
		readWriteByte(*data_in);
		data_in++;
	}
}


uint8_t SPI_lib::readByte(void)
{
	while (((this->SPI)->SR & SPI_I2S_FLAG_RXNE) == RESET) ;	//wait data send end
//	*((__IO uint8_t *)&SPI->DR) = Data;
	return ((this->SPI)->DR);
}

uint8_t SPI_lib::readWriteByte(uint8_t data)
{
	writeByte(data);
	return readByte();
}

uint16_t SPI_lib::readWriteHalfWord(uint16_t data)
{
	uint16_t resp = 0;
	
	resp <<= 8;
	resp |= readWriteByte((uint8_t)(data >> 8));
	resp <<= 8;
	resp |= readWriteByte((uint8_t)data);
	
	return resp;
}

uint32_t SPI_lib::readWriteWord(uint32_t data)
{
	uint32_t resp = 0;
	
	resp <<= 8;
	resp |= readWriteByte((uint8_t)(data >> 24));
	resp <<= 8;
	resp |= readWriteByte((uint8_t)(data >> 16));
	resp <<= 8;
	resp |= readWriteByte((uint8_t)(data >> 8));
	resp <<= 8;
	resp |= readWriteByte((uint8_t)data);
	
	return resp;
}


void SPI_lib::setSpeedLow(void)
{
	//SPI_Cmd(SPI, DISABLE);
	spi_config.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
	SPI_Init(SPI, &spi_config);
	SPI_Cmd(SPI, ENABLE);
}

void SPI_lib::setSpeedHigh(void)
{
	spi_config.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_Init(SPI, &spi_config);
	SPI_Cmd(SPI, ENABLE);
}
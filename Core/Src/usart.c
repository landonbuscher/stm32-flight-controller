#include "usart.h"
#include "stm32f4xx.h"

#define RX_LEN 128
static volatile uint8_t rx_buffer[RX_LEN];
static volatile uint16_t bytes_read = 0;

//Configure & enable USART with DMA for logging
void usart_init(void) {
	//USART1_TX - PA9, DMA2 CH4 ST7
	//USART1_RX - PA10, DMA2 CH4 ST5

	//Enable GPIO/USART/DMA
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;

	//Set alternate function mode
	GPIOA->MODER &= ~(GPIO_MODER_MODER9 | GPIO_MODER_MODER10);
	GPIOA->MODER |= (GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1);

	//Set pull-up
	GPIOA->PUPDR |= GPIO_PUPDR_PUPD9_0;
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD9_1;

	//Set alternate function value
	GPIOA->AFR[1] &= ~(0b1111 << 4 | 0b1111u << 8);
	GPIOA->AFR[1] |=  (7 << 4 | 7 << 8);

	//BRR = 460,800 baud
	USART1->BRR = 11<<4 | 6;

	//Enable transmitter & receiver, 8-bit word length, disable parity control, enable IDLE interrupt
	USART1->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_IDLEIE;
	USART1->CR1 &= ~(USART_CR1_M | USART_CR1_PCE);
	USART1->CR2 &= ~(USART_CR2_STOP);

	//Ensure DMA disabled for configuration
	DMA2_Stream5->CR &= ~DMA_SxCR_EN;
	DMA2_Stream7->CR &= ~DMA_SxCR_EN;
	while((DMA2_Stream5->CR & DMA_SxCR_EN) || (DMA2_Stream7->CR & DMA_SxCR_EN)) {}

	//Enable DMA NVIC
	NVIC_EnableIRQ(DMA2_Stream5_IRQn);
	NVIC_EnableIRQ(USART1_IRQn);

	NVIC_SetPriority(DMA2_Stream5_IRQn, 0);
	NVIC_SetPriority(USART1_IRQn, 0);

	//DMA Configuration
	DMA2_Stream5->CR = (DMA_SxCR_CHSEL_2 | DMA_SxCR_PL_1 | DMA_SxCR_MINC | DMA_SxCR_HTIE | DMA_SxCR_CIRC);
	DMA2_Stream7->CR = (DMA_SxCR_CHSEL_2 | DMA_SxCR_PL_1 | DMA_SxCR_MINC | DMA_SxCR_DIR_0);

	DMA2_Stream5->PAR = (uint32_t)&USART1->DR;
	DMA2_Stream7->PAR = (uint32_t)&USART1->DR;

	DMA2_Stream5->M0AR = (uint32_t)rx_buffer;
	DMA2_Stream5->NDTR = RX_LEN;

	USART1->CR3 |= USART_CR3_DMAT | USART_CR3_DMAR;

	//Enable USART
	USART1->CR1 |= USART_CR1_UE;

	//Enable USART_RX DMA
	DMA2_Stream5->CR |= DMA_SxCR_EN;
}

void usart_tx(uint8_t* buffer, uint16_t len) {
	if (DMA2_Stream7->CR & DMA_SxCR_EN) return; //DMA busy - TODO add queue
	DMA2->HIFCR = DMA_HIFCR_CTCIF7 | DMA_HIFCR_CHTIF7 | DMA_HIFCR_CTEIF7 | DMA_HIFCR_CDMEIF7 | DMA_HIFCR_CFEIF7;
	DMA2_Stream7->M0AR = (uint32_t)buffer;
	DMA2_Stream7->NDTR = len;
	DMA2_Stream7->CR |= DMA_SxCR_EN;
}

static void usart_rx_handler(uint16_t start_idx, uint16_t len) {
	usart_tx((uint8_t*)&rx_buffer[start_idx], len);
}

void usart_rx(void) {
	uint16_t bytes_received = RX_LEN - DMA2_Stream5->NDTR;
	if (bytes_received == bytes_read) {
		return;
	} else if (bytes_received > bytes_read) {
		//Handle data in range [bytes_read, bytes_received]
		usart_rx_handler(bytes_read, bytes_received - bytes_read);
	} else {
		//bytes_received wrapped back around, need two handlers
		usart_rx_handler(bytes_read, RX_LEN - bytes_read);
		usart_rx_handler(0, bytes_received);
	}
	bytes_read = bytes_received;
}

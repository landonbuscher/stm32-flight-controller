#include "usart.h"
#include "stm32f4xx.h"

//Configure & enable USART for logging
void usart_init(void) {
	//PA9 - USART1_TX
	//PA10 - USART1_RX

	//Enable GPIO/USART
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

	//Set alternate function mode
	GPIOA->MODER |= GPIO_MODER_MODER9_1;
	GPIOA->MODER &= ~GPIO_MODER_MODER9_0;

	//Set pull-up
	GPIOA->PUPDR |= GPIO_PUPDR_PUPD9_0;
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD9_1;

	//Set alternate function value
	GPIOA->AFR[1] |= 7<<4;

	//BRR = 460,800 baud
	USART1->BRR = 11<<4 | 6;

	//Enable transmitter, 8-bit word length, disable parity control
	USART1->CR1 |= USART_CR1_TE;
	USART1->CR1 &= ~(USART_CR1_M | USART_CR1_PCE);
	USART1->CR2 &= ~(USART_CR2_STOP);

	//Enable USART
	USART1->CR1 |= USART_CR1_UE;
}

void usart_send(uint8_t byte) {
	while (!(USART1->SR & USART_SR_TXE)) {}
	USART1->DR = byte;
}

void usart_print(char *s) {
	while (*s) usart_send(*s++);
}

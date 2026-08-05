#include "spi.h"
#include "stm32f4xx.h"

void spi_init(void) {
	//Enable SPI2 (IMU, Baro on SPI2)
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	 //Set baud rate to f_plk/8 (5.25MHz), 8-bit data frame, MSB first
	SPI2->CR1 &= ~(SPI_CR1_BR
			| SPI_CR1_DFF
			| SPI_CR1_LSBFIRST);
	//Set SPI mode 3, software slave management + tie NSS high, master mode, enable SPI
	SPI2->CR1 |= SPI_CR1_BR_1 | SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR | SPI_CR1_SPE;

	//PB13 - S2_CLK
	//PB14 - S2_SDO
	//PB15 - S2_SDI

	//Set AF mode on PB13,14,15
	GPIOB->MODER |= GPIO_MODER_MODER13_1 | GPIO_MODER_MODER14_1 | GPIO_MODER_MODER15_1;
	GPIOB->MODER &= ~(GPIO_MODER_MODER13_0 | GPIO_MODER_MODER14_0 | GPIO_MODER_MODER15_0);

	//Set high speed on PB13,14,15
	GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEED13_1 | GPIO_OSPEEDR_OSPEED14_1 | GPIO_OSPEEDR_OSPEED15_1;
	GPIOB->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED13_0 | GPIO_OSPEEDR_OSPEED14_0 | GPIO_OSPEEDR_OSPEED15_0);

	//Set AF5 on PB13,14,15
	GPIOB->AFR[1] |= 5<<20 | 5<<24 | 5<<28;
}

//Cycle one byte
static uint8_t spi_transfer(uint8_t byte) {
	SPI2->DR = byte;
	while (!(SPI2->SR & SPI_SR_RXNE)) {}
	return SPI2->DR;
}

//Read single byte
uint8_t spi_read(Peripheral* peripheral, uint8_t register_addr) {
	uint8_t data;
	peripheral->select();
	(void)spi_transfer(register_addr | (1<<7));
	data = spi_transfer(0x00);
	while (SPI2->SR & SPI_SR_BSY) {}
	peripheral->deselect();
	return data;
}

//Read several bytes
void spi_read_burst(Peripheral* peripheral, uint8_t start_addr, uint8_t count, uint8_t* buffer) {
	peripheral->select();
	(void)spi_transfer(start_addr | (1<<7));
	for (int i=0; i<count; i++) {
		buffer[i] = spi_transfer(0x00);
	}
	while (SPI2->SR & SPI_SR_BSY) {}
	peripheral->deselect();
}

//Write byte
void spi_write(Peripheral* peripheral, uint8_t register_addr, uint8_t data) {
	peripheral->select();
	(void)spi_transfer(register_addr);
	(void)spi_transfer(data);
	while (SPI2->SR & SPI_SR_BSY) {}
	peripheral->deselect();
}

#include "lps22hh.h"
#include "stm32f4xx.h"

Peripheral lps22hh_peripheral = {
		.select = lps22hh_spi_select,
		.deselect = lps22hh_spi_deselect
};

void lps22hh_init(void) {
	//ENABLE GPIOC
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	(void)RCC->AHB1ENR; //Make sure write goes through before configuration

	//CONFIGURE OUTPUT MODE ON PC13
	GPIOC->MODER |= GPIO_MODER_MODER13_0;
	GPIOC->MODER &= ~GPIO_MODER_MODER13_1;

	//MAKE SURE PIN IS DESELECTED
	GPIOC->BSRR = GPIO_BSRR_BS_13;
}

void lps22hh_config(void) {
	spi_write(&lps22hh_peripheral, 0x11, 0b100); //reset
	while (spi_read(&lps22hh_peripheral, 0x11) & 0b100) {} //wait for reset
	spi_write(&lps22hh_peripheral, 0x10, 0b1011010); //75Hz output rate, enable BDU, configure LPF
	spi_write(&lps22hh_peripheral, 0x11, 0b10010); //enable register addr auto increment, LPF
}

void lps22hh_spi_select(void) {
	GPIOC->BSRR = GPIO_BSRR_BR_13;
}

void lps22hh_spi_deselect(void) {
	GPIOC->BSRR = GPIO_BSRR_BS_13;
}

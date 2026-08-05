#include "stm32f4xx.h"

//Enable 84MHz PLL
void clock_init(void) {
	//Configure latency
	FLASH->ACR |= 2; //2WS required at 84MHz HCLK & 3.3v per datasheet

	//Enable HSI
	RCC->CR |= RCC_CR_HSION;
	while (!(RCC->CR & RCC_CR_HSIRDY)) {}

	//Configure PLL
	RCC->PLLCFGR = 16 | 336 << 6 | 1 << 16 | 4<<24 | RCC_PLLCFGR_PLLSRC_HSI; // M=/16, P=/4, N=*336, HSI

	//Configure AHB/APB clocks (84MHz for AHB/APB2, 42MHz for APB1)
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV1 | RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_HPRE_DIV1;

	//Enable and switch system clock to PLL
	RCC->CR |= RCC_CR_PLLON;
	while (!(RCC->CR & RCC_CR_PLLRDY)) {}

	RCC->CFGR |= RCC_CFGR_SW_PLL;
	while (!(RCC->CFGR & RCC_CFGR_SWS_PLL)) {}

	//Update SYSCLK and SysTick
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock/1000);
}

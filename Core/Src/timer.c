#include "timer.h"
#include "stm32f4xx.h"

//Enable 1kHz control loop
void timer_tim2_init(void) { //Fires every millisecond
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; //enable tim2
	TIM2->CR1 |= TIM_CR1_ARPE; //set auto reload preload enable
	//Generates 1kHz clock from 84MHz peripheral clock
	TIM2->PSC = 1;
	TIM2->ARR = 41999;
	TIM2->DIER |= TIM_DIER_UIE; //enable interrupts on overflow
	TIM2->EGR |= TIM_EGR_UG;
	TIM2->SR = ~TIM_SR_UIF;

	//Enable TIM2 and interrupts
	NVIC_SetPriority(TIM2_IRQn, 0);
	NVIC_EnableIRQ(TIM2_IRQn);
	TIM2->CR1 |= TIM_CR1_CEN;
}

void timer_tim5_init(void) { //Fires every microsecond
	RCC->APB1ENR |= RCC_APB1ENR_TIM5EN; //enable tim5
	TIM5->CR1 |= TIM_CR1_ARPE; //set auto reload preload enable
	//Generates 1MHz clock from 84MHz peripheral clock
	TIM5->PSC = 83;
	TIM5->ARR = 0xFFFFFFFF;
	TIM5->EGR |= TIM_EGR_UG;
	TIM5->SR = ~TIM_SR_UIF;

	//Enable TIM5
	TIM5->CR1 |= TIM_CR1_CEN;
}

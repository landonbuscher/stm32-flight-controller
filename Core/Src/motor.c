#include "motor.h"
#include "stm32f4xx.h"

//Configures & enables 20kHz PWM signal
void motor_init(void) {
	// MOTOR 1..4 = PB 6..9 (TIM4 CH1-4 AF02)
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	//set alternate function mode
	GPIOB->MODER |= GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1 | GPIO_MODER_MODER8_1 | GPIO_MODER_MODER9_1;
	GPIOB->MODER &= ~(GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0);
	//set push pull
	GPIOB->OTYPER &= ~(GPIO_OTYPER_OT6 | GPIO_OTYPER_OT7 | GPIO_OTYPER_OT8 | GPIO_OTYPER_OT9);
	//set high speed
	GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEED6_1 | GPIO_OSPEEDR_OSPEED7_1 | GPIO_OSPEEDR_OSPEED8_1 | GPIO_OSPEEDR_OSPEED9_1;
	GPIOB->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED6_0 | GPIO_OSPEEDR_OSPEED7_0 | GPIO_OSPEEDR_OSPEED8_0 | GPIO_OSPEEDR_OSPEED9_0);
	//set no pupd
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR6 | GPIO_PUPDR_PUPDR7 | GPIO_PUPDR_PUPDR8 | GPIO_PUPDR_PUPDR9);
	//set AFRH/L as TIM4
	GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL6 | GPIO_AFRL_AFSEL7);
	GPIOB->AFR[1] &= ~(GPIO_AFRH_AFSEL8 | GPIO_AFRH_AFSEL9);
	GPIOB->AFR[0] |= (2<<24) | (2<<28);
	GPIOB->AFR[1] |= (2) | (2<<4);

	//enable TIM4 + set auto reload preload enable
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
	TIM4->CR1 |= TIM_CR1_ARPE;
	//configure timer (20kHz)
	TIM4->PSC = 0;
	TIM4->ARR = 4199;
	//set PWM mode 1 and capture compare preload enable
	TIM4->CCMR1 |= TIM_CCMR1_OC1PE | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2; //PWM MODE 1 (ACTIVE WHEN CNT<CCR1)
	TIM4->CCMR1 |= TIM_CCMR1_OC2PE | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2;
	TIM4->CCMR2 |= TIM_CCMR2_OC3PE | TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2;
	TIM4->CCMR2 |= TIM_CCMR2_OC4PE | TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2;
	//reset CCR registers
	TIM4->CCR1 = 0;
	TIM4->CCR2 = 0;
	TIM4->CCR3 = 0;
	TIM4->CCR4 = 0;
	//enable CCR registers
	TIM4->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;
	//force update of registers
	TIM4->EGR |= TIM_EGR_UG;
	//enable timer
	TIM4->CR1 |= TIM_CR1_CEN;
}

//Send equal output to all motors (debugging)
void motor_global_output(float percent) {
	if (percent>1) percent=1;
	if (percent<0) percent=0;
	TIM4->CCR1 = (uint16_t)(percent*TIM4->ARR);
	TIM4->CCR2 = (uint16_t)(percent*TIM4->ARR);
	TIM4->CCR3 = (uint16_t)(percent*TIM4->ARR);
	TIM4->CCR4 = (uint16_t)(percent*TIM4->ARR);
}

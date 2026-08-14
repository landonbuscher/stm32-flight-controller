#include <math.h>
#include "rc.h"
#include "usart.h"
#include "pid.h"
#include "stm32f4xx.h"

static volatile uint32_t prev;
static volatile uint8_t channel;
static volatile uint16_t channel_odr[8];

static volatile uint16_t log_prescaler = 100;
static volatile uint16_t log_counter = 0;


//All range from 1000 (L)-2000 (H) us
//CH1 right stick left (L)/right (H)		ROLL
//CH2 right stick up (L)/down (H)			PITCH
//CH3 left stick up (H)/down (L)			THROTTLE
//CH4 left stick left (L)/right (H)			YAW
//CH5 right rocker (back L, forward H)		ARM
//CH6 right button (pressed H, default L)	N/A
//CH7 left rocker (back L, forward H)		N/A
//CH8 potentiometer (left L, right H)		PID TUNER

static void rc_input_handler(void) {
	//Log Data
	if (log_counter >= log_prescaler) {
		log_counter=0;
		static char str[72];
		uint16_t len = snprintf(str, 72, "%d %d %d %d %d %d %d %d\n",
				channel_odr[0], channel_odr[1], channel_odr[2], channel_odr[3], channel_odr[4], channel_odr[5], channel_odr[6], channel_odr[7]);
		if (len>72) len = 72;
		usart_tx((uint8_t*)str, len);
	}

	if (channel_odr[4] > 1750) {
		//0-100 output
		pid_arm();
		float roll = ((channel_odr[0]-1000)/10.0f-50)/2.0f; //Divide by 5 = +-50deg -> +- 10deg control authority
		float pitch  = ((channel_odr[1]-1000)/10.0f-50)/2.0f;
		float throttle = (channel_odr[2]-1000)/10.0f;
		float yaw = (channel_odr[3]-1500)/5.0f; //-100 to 100 deg/s
		pid_set_throttle(throttle);
		pid_set_pitch_setpoint(pitch);
		pid_set_roll_setpoint(roll);
		if (fabsf(yaw) >= 3) {
			pid_set_yaw_setpoint(yaw);
		} else {
			pid_set_yaw_setpoint(0.0f);
		}
	} else {
		pid_disarm();
	}
}

void EXTI0_IRQHandler(void) {
	EXTI->PR = EXTI_PR_PR0;

	uint32_t now = TIM5->CNT;
	uint32_t dt = now-prev;
	prev = now;

	if (dt>3000) {
		channel = 0;
	} else if (channel < 8) {
		if (dt>800 && dt<2200) {
			channel_odr[channel] = (uint16_t)dt;
		}
		channel++;
	}

	rc_input_handler();

	log_counter++;
}

void rc_init(void) {
	//RCC Configuration
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	//Input
	GPIOA->MODER &= ~(GPIO_MODER_MODER0);
	//Push pull
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT0;
	//High Speed
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED0_1;
	GPIOA->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED0_0;
	//No pupd
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD0;
	//Set PA0 for EXTI0
	SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI0;
	//Enable EXTI0, trigger on rising edge, don't trigger on falling edge
	EXTI->IMR |= EXTI_IMR_MR0;
	EXTI->RTSR |= EXTI_RTSR_TR0;
	EXTI->FTSR &= ~EXTI_FTSR_TR0;
	//Enable interrupt
	NVIC_EnableIRQ(EXTI0_IRQn);
	NVIC_SetPriority(EXTI0_IRQn, 2);
}

#include <string.h>
#include <stdio.h>
#include <math.h>

#include "main.h"
#include "spi.h"
#include "lsm6dsr.h"
#include "timer.h"
#include "clock.h"
#include "usart.h"
#include "systick.h"
#include "motor.h"
#include "pid.h"
#include "rc.h"

//CONSTANTS
#define DEG_TO_RAD (3.14159265f/180.0f)
#define RAD_TO_DEG (180.0f/3.14159265f)

//COMPLEMENTARY FILTER
#define TAU 0.5f
#define ALPHA ((TAU)/(TAU+DT))

//GLOBAL VARIABLES
static volatile float xl_roll, xl_pitch;
static volatile float gx, gy, gz, ax, ay, az;
static volatile float pitch, roll;
volatile uint8_t log_ready = 0, log_count = 0;

// INTERRUPTS
// Control timer running at 1kHz with log_count to step it down to 100Hz for USART logging
void TIM2_IRQHandler(void) {
	TIM2->SR = ~TIM_SR_UIF;
	float buffer[6];
	lsm6dsr_get_gyro_xl_phys(buffer);
	gx = buffer[0]*DEG_TO_RAD, gy = -buffer[1]*DEG_TO_RAD, gz = -buffer[2]*DEG_TO_RAD;
	ax = buffer[3], ay = buffer[4], az = buffer[5];

	xl_roll = atan2f(ay, az);
	xl_pitch = atan2f(ax, sqrtf(ay*ay + az*az));

	roll = ALPHA*(roll+gx*DT) + (1-ALPHA)*xl_roll; //rad
	pitch = ALPHA*(pitch+gy*DT) + (1-ALPHA)*xl_pitch; //rad

	if (pid_get_armed()) {
		pid_handler(pitch*RAD_TO_DEG, roll*RAD_TO_DEG, gy*RAD_TO_DEG, gx*RAD_TO_DEG, gz*RAD_TO_DEG);
	}

	if (++log_count >= 10) { //1000Hz -> 100Hz for USART logging
		log_ready = 1;
		log_count = 0;
	}
}

void USART1_IRQHandler(void) {
	(void)USART1->SR;
	(void)USART1->DR;
	usart_rx();
}

void DMA2_Stream5_IRQHandler(void) {
	if (DMA2->HISR & DMA_HISR_HTIF5) {
		DMA2->HIFCR = DMA_HIFCR_CHTIF5;
		usart_rx();
	}
	if (DMA2->HISR & DMA_HISR_TCIF5) {
		DMA2->HIFCR = DMA_HIFCR_CTCIF5;
		usart_rx();
	}
}

void DMA2_Stream7_IRQHandler(void) {
	DMA2->HIFCR = DMA_HIFCR_CTCIF7;
	usart_tx_queue_handler();
}

int main(void) {
	//POWER ON & CONFIGURATION
	clock_init(); //Enable 84MHz PLL SYSCLK
	spi_init(); //Enable SPI
	lsm6dsr_init(); //Enable IMU
	lsm6dsr_config(); //Configure IMU
	usart_init(); //Enable USART
	motor_init(); //Enable + configure motor output

	const char pwr_msg[] = "# Power On & Configuration Complete\n";
	usart_tx((uint8_t*)pwr_msg, sizeof(pwr_msg)-1);

	//STARTUP CALIBRATION
	lsm6dsr_calibration();
	const char calib_msg[] = "# Calibration Complete\n";
	usart_tx((uint8_t*)calib_msg, sizeof(calib_msg)-1);

	//SETUP TIMERS
	timer_tim2_init(); //Enable control loop and start recording data
	timer_tim5_init(); //Enable 1MHz clock for RC input

	//Enable RC input
	rc_init();

	while (1) {
		if (log_ready) {
			//Send pitch and roll data via USART
			log_ready = 0;
//		    static char str[80];
////		    uint16_t len = snprintf(str, 80, "%lu %.2f\n", systick_get_ms(), motor_get(FL)->output);
//		    uint16_t len = snprintf(str, 80, "%lu %.5f %.5f\n", systick_get_ms(), pitch, roll);
//		    if (len>80) len = 80;
//		    usart_tx((uint8_t*)str, len);
		}
		__WFI();
	}
}

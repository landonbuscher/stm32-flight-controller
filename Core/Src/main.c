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

//CONSTANTS
#define DEG_TO_RAD (3.14159265f/180.0f)
#define DT (1.0f/1005.0f) //Control timer set to 1000Hz; True measured is 1005Hz

//COMPLEMENTARY FILTER
#define TAU 0.5f
#define ALPHA (TAU)/(TAU+DT)

//GLOBAL VARIABLES
static volatile float xl_roll, xl_pitch;
static volatile float gx, gy, gz, ax, ay, az;
static volatile float tx, ty, tz;
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

	tx = ALPHA*(tx+gx*DT) + (1-ALPHA)*xl_roll;
	ty = ALPHA*(ty+gy*DT) + (1-ALPHA)*xl_pitch;

	if (++log_count >= 10) { //1000Hz -> 100Hz for USART logging
		log_ready = 1;
		log_count = 0;
	}
}

void USART1_IRQHandler(void) { usart_rx(); }

void DMA2_Stream5_IRQHandler(void) { usart_rx(); }

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

	//START LOGGING
	timer_init(); //Enable control loop and start recording data

//	motor_output(FR, 0.1);

	while (1) {
		if (log_ready) {
			//Send pitch and roll data via USART
			log_ready = 0;
		    static char str[80];
		    uint16_t len = snprintf(str, 80, "%lu %.2f %.2f\n", systick_get_ms(), tx, ty);
		    if (len<0) continue;
		    if (len>80) len = 80;
		    usart_tx((uint8_t*)str, len);
		}
		__WFI();
	}
}

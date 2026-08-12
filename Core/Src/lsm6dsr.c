#include "lsm6dsr.h"
#include "stm32f4xx.h"
#include "usart.h"
#include "systick.h"

//CONSTANTS
#define GYRO_SCALE (70.0f/1000.0f) //Valid @ +-2000dps
#define XL_SCALE (0.244f/1000.0f) //Valid @ +-8g

// CONFIGURATION
#define CALIBRATION_DURATION_MS 2500
#define SETTLE_TIME_MS 5000

//GLOBAL VARIABLES
Peripheral lsm6dsr_peripheral = {
		.select = lsm6dsr_spi_select,
		.deselect = lsm6dsr_spi_deselect
};

static float GYRO_X_CAL, GYRO_Y_CAL, GYRO_Z_CAL;

//XL calibration values obtained by measuring XL readouts at known orientations
static float XL_X_OFFSET = 0.023604970017f;
static float XL_Y_OFFSET = -0.006748496689f;
static float XL_Z_OFFSET = 0.014031421250000009f;
static float XL_X_SCALE = 0.9924893666666667f;
static float XL_Y_SCALE = 1.001926015625f;
static float XL_Z_SCALE = 1.0062408837499999f;

void lsm6dsr_init(void) {
	//ENABLE GPIOA
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	(void)RCC->AHB1ENR; //Make sure write goes through before configuration

	//CONFIGURE OUTPUT MODE ON PA8
	GPIOA->MODER |= GPIO_MODER_MODER8_0;
	GPIOA->MODER &= ~GPIO_MODER_MODER8_1;

	//MAKE SURE PIN IS DESELECTED
	GPIOA->BSRR = GPIO_BSRR_BS_8;
}

void lsm6dsr_config(void) {
	spi_write(&lsm6dsr_peripheral, 0x12, 0b1); //reset
	while (spi_read(&lsm6dsr_peripheral, 0x12) & 0b1) {} //wait for reset to be done
	spi_write(&lsm6dsr_peripheral, 0x12, 0b1000100); //set BDU
	spi_write(&lsm6dsr_peripheral, 0x11, 0b1000<<4 | 0b11<<2); //turn on gyro, 1.66kHz, set +-2000dps
	spi_write(&lsm6dsr_peripheral, 0x10, 0b1000<<4 | 0b11<<2); //turn on accel, 1.66kHz, set +-8g
}

void lsm6dsr_spi_select(void) {
	GPIOA->BSRR = GPIO_BSRR_BR_8;
}


void lsm6dsr_spi_deselect(void) {
	GPIOA->BSRR = GPIO_BSRR_BS_8;
}

void lsm6dsr_get_gyro(int16_t* buffer) {
	uint8_t data[6];
	spi_read_burst(&lsm6dsr_peripheral, 0x22, 6, data);
	buffer[0] = data[1]<<8 | data[0];
	buffer[1] = data[3]<<8 | data[2];
	buffer[2] = data[5]<<8 | data[4];
}

void lsm6dsr_get_xl(int16_t* buffer) {
	uint8_t data[6];
	spi_read_burst(&lsm6dsr_peripheral, 0x28, 6, data);
	buffer[0] = data[1]<<8 | data[0];
	buffer[1] = data[3]<<8 | data[2];
	buffer[2] = data[5]<<8 | data[4];
}

void lsm6dsr_get_gyro_xl(int16_t* buffer) {
	uint8_t data[12];
	spi_read_burst(&lsm6dsr_peripheral, 0x22, 12, data);
	buffer[0] = data[1]<<8 | data[0];
	buffer[1] = data[3]<<8 | data[2];
	buffer[2] = data[5]<<8 | data[4];
	buffer[3] = data[7]<<8 | data[6];
	buffer[4] = data[9]<<8 | data[8];
	buffer[5] = data[11]<<8 | data[10];
}

void lsm6dsr_get_gyro_phys(float* buffer) {
	int16_t data[3];
	lsm6dsr_get_gyro(data);
	buffer[0] = data[0]*GYRO_SCALE-GYRO_X_CAL;
	buffer[1] = data[1]*GYRO_SCALE-GYRO_Y_CAL;
	buffer[2] = data[2]*GYRO_SCALE-GYRO_Z_CAL;
}

void lsm6dsr_get_xl_phys(float* buffer) {
	int16_t data[3];
	lsm6dsr_get_xl(data);
	buffer[0] = (data[0]*XL_SCALE-XL_X_OFFSET)/XL_X_SCALE;
	buffer[1] = (data[1]*XL_SCALE-XL_Y_OFFSET)/XL_Y_SCALE;
	buffer[2] = (data[2]*XL_SCALE-XL_Z_OFFSET)/XL_Z_SCALE;
}

void lsm6dsr_get_gyro_xl_phys(float* buffer) {
	int16_t data[6];
	lsm6dsr_get_gyro_xl(data);
	buffer[0] = data[0]*GYRO_SCALE-GYRO_X_CAL;
	buffer[1] = data[1]*GYRO_SCALE-GYRO_Y_CAL;
	buffer[2] = data[2]*GYRO_SCALE-GYRO_Z_CAL;
	buffer[3] = (data[3]*XL_SCALE-XL_X_OFFSET)/XL_X_SCALE;
	buffer[4] = (data[4]*XL_SCALE-XL_Y_OFFSET)/XL_Y_SCALE;
	buffer[5] = (data[5]*XL_SCALE-XL_Z_OFFSET)/XL_Z_SCALE;
}

void lsm6dsr_calibration(void) {

	uint32_t start = systick_get_ms();
	uint32_t now = start;
	uint16_t logs = 0;
	int16_t data[3] = {0};
	float sums[3] = {0};

	//Wait SETTLE_TIME_MS before calibrating
	while (now-start<SETTLE_TIME_MS) {
		now = systick_get_ms();
	}

	start = now;
	uint32_t last_log = start;

	//CALIBRATION_DURATION_MS seconds to collect data
	while (now-start<CALIBRATION_DURATION_MS) {
		if (now-last_log >= 10) {
			lsm6dsr_get_gyro(data);
			for (int i=0; i<3; i++) {
				sums[i] += data[i]*GYRO_SCALE;
			}
			last_log += 10;
			logs++;
		}
		now = systick_get_ms();
	}

	//Calculate & return bias offset
	float offsets[3] = {0};
	for (int i=0; i<3; i++) {
		offsets[i] = sums[i]/logs;
	}

	GYRO_X_CAL = offsets[0];
	GYRO_Y_CAL = offsets[1];
	GYRO_Z_CAL = offsets[2];
}

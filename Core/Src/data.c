//#include <string.h>
//#include <stdio.h>
//
//#include "main.h"
//#include "spi.h"
//#include "usart.h"
//#include "lsm6dsr.h"
//#include "lps22hh.h"
//#include "timer.h"
//
////volatile uint16_t bat;
////volatile uint16_t vref_mv;
////volatile uint16_t vref_calib;
////volatile uint16_t vref_dr;
//
//volatile int16_t ax, ay, az, gx, gy, gz;
//volatile uint32_t pres;
//volatile int32_t temp;
//
//volatile uint8_t imu_raw[12];
//volatile uint8_t baro_raw[5];
//volatile uint8_t xl_g_status;
//volatile uint8_t baro_status;
//volatile uint16_t xl_g_stale=0;
//volatile uint16_t baro_stale=0;
//volatile uint8_t xl_g_sample_rdy=0;
//volatile uint8_t baro_sample_rdy=0;
//
//volatile uint32_t millis = 0;
//static uint32_t last_log = 0;
//
//volatile uint8_t whoami;
//
//Peripheral lsm6dsr_peripheral = {
//		.select = lsm6dsr_spi_select,
//		.deselect = lsm6dsr_spi_deselect
//};
//
//Peripheral lps22hh_peripheral = {
//		.select = lps22hh_spi_select,
//		.deselect = lps22hh_spi_deselect
//};
//
//void TIM2_IRQHandler(void) {
//	TIM2->SR &= ~TIM_SR_UIF;
//	//xl/gyro read
//	xl_g_status = spi_read(&lsm6dsr_peripheral, 0x1E);
//	if (!(xl_g_status & 0b11)) xl_g_stale++;
//	spi_read_burst(&lsm6dsr_peripheral, 0x22, 12, (uint8_t*)imu_raw);
//	xl_g_sample_rdy=1;
//
//	//baro read
//	baro_status = spi_read(&lps22hh_peripheral, 0x27);
//	if (!(baro_status & 0b11)) baro_stale++;
//	spi_read_burst(&lps22hh_peripheral, 0x28, 5, (uint8_t*)baro_raw);
//	baro_sample_rdy=1;
//}
//
//int main(void) {
//
//	spi_init();
//	usart_init();
//	lsm6dsr_init();
//	lps22hh_init();
//	timer_init();
//
//	//Configure LSM6DSR
//	spi_write(&lsm6dsr_peripheral, 0x12, 0b1); //reset
//	while (spi_read(&lsm6dsr_peripheral, 0x12) & 0b1) {} //wait for reset to be done
//	spi_write(&lsm6dsr_peripheral, 0x12, 0b1000100); //set BDU
////	spi_write(&lsm6dsr_peripheral, 0x11, 0b1000<<4 | 0b11<<2); //turn on gyro, 1.66kHz, set +-2000dps
//	spi_write(&lsm6dsr_peripheral, 0x11, (0b1000<<4 & ~(0b11)<<2) | (1 << 1)); //turn on gyro, 1.66kHz, set +-125
////	spi_write(&lsm6dsr_peripheral, 0x10, 0b1000<<4 | 0b11<<2); //turn on accel, 1.66kHz, set +-8g
//	spi_write(&lsm6dsr_peripheral, 0x10, 0b1000<<4 & ~(0b11)<<2); //turn on accel, 1.66kHz, set +-2g
//
//
//	//Configure LPS22HH
//	spi_write(&lps22hh_peripheral, 0x11, 0b100); //reset
//	while (spi_read(&lps22hh_peripheral, 0x11) & 0b100) {} //wait for reset
//	spi_write(&lps22hh_peripheral, 0x10, 0b1011010); //75Hz output rate, enable BDU, configure LPF
//	spi_write(&lps22hh_peripheral, 0x11, 0b10010); //enable register addr auto increment, LPF
//
//	//Enable TIM2 and interrupts
//	NVIC_SetPriority(TIM2_IRQn, 0);
//	NVIC_EnableIRQ(TIM2_IRQn);
//	TIM2->CR1 |= TIM_CR1_CEN;
//
////	usart_print("# ms, pres_raw, temp_raw\n");
//	usart_print("# ms, ax, ay, az, gx, gy, gz, temp, pres\n");
////	usart_print("# ms, temp\n");
//
//	while (1) {
//		if (xl_g_sample_rdy) {
//			uint8_t imu_buf[12];
//			__disable_irq();
//			//Disable interrupt handler and copy imu_raw into a buffer so that imu data is
//			//not changed mid-conversion
//			memcpy(imu_buf, (const void*)imu_raw, 12);
//			xl_g_sample_rdy=0;
//			__enable_irq();
//			//unit: g's, dps
//			ax = (int16_t)((imu_buf[7]<<8) | imu_buf[6]);
//			ay = (int16_t)((imu_buf[9]<<8) | imu_buf[8]);
//			az = (int16_t)((imu_buf[11]<<8) | imu_buf[10]);
//
//			gx = (int16_t)((imu_buf[1]<<8) | imu_buf[0]);
//			gy = (int16_t)((imu_buf[3]<<8) | imu_buf[2]);
//			gz = (int16_t)((imu_buf[5]<<8) | imu_buf[4]);
//		}
//		if (baro_sample_rdy) {
//			uint8_t baro_buf[5];
//			__disable_irq();
//			memcpy(baro_buf, (const void*)baro_raw, 5);
//			baro_sample_rdy=0;
//			__enable_irq();
//			pres = (uint32_t)((baro_buf[2]<<16) | (baro_buf[1]<<8) | (baro_buf[0]));
//			temp = (uint16_t)((baro_buf[4]<<8) | baro_buf[3]);
//
//		}
//		if (millis - last_log >= 10) {
//			last_log+=10;
//			char buf[128];
////			snprintf(buf, sizeof buf, "%lu, %lu, %lu\n", millis, pres, temp);
//			snprintf(buf, sizeof buf, "%lu, %d, %d, %d, %d, %d, %d, %ld, %ld\n", millis, ax, ay, az, gx, gy, gz, temp, pres);
////			snprintf(buf, sizeof buf, "%lu, %ld\n", millis, temp);
//			usart_print(buf);
//		}
//		__WFI(); //wait for interrupt before checking again
//	}
//
//	//BATTERY LEVEL SENSING
//
////	// Turn on HSI clock and await HSIRDY
////	RCC->CR |= RCC_CR_HSION;
////	while(!(RCC->CR & RCC_CR_HSIRDY)) {}
////
////	//Turn on ADC clock
////	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
////
////	//Turn on ADC
////	ADC1->CR2 |= ADC_CR2_ADON;
////
////	//Configure 1 read
////	ADC1->SQR1 &= ~(ADC_SQR1_L_3 | ADC_SQR1_L_2 | ADC_SQR1_L_1 | ADC_SQR1_L_0);
////
////	//Set up VREFINT read
////	ADC1->SMPR1 |= ADC_SMPR1_SMP17;
////	ADC1->SQR3 &= ~ADC_SQR3_SQ1;
////	ADC1->SQR3 |= 17; //VREFINT
////
////	//Get vref calibration
////	vref_calib = *((uint16_t*) 0x1FFF7A2A);
////
////	//Read from VREFINT
////	ADC->CCR |= ADC_CCR_TSVREFE;
////	for (volatile int i=0; i<50000; i++) {}
////	ADC1->CR2 |= ADC_CR2_SWSTART;
////	while (!(ADC1->SR & ADC_SR_EOC)) {}
////	vref_dr = ADC1->DR;
////	vref_mv = 3300*vref_calib/vref_dr;
////	ADC->CCR &= ~ADC_CCR_TSVREFE;
////
////	//Set up VBAT read
////	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
////	GPIOB->MODER |= GPIO_MODER_MODER1_1 | GPIO_MODER_MODER1_0;
////	ADC1->SMPR2 |= ADC_SMPR2_SMP9;
////	ADC1->SQR3 &= ~ADC_SQR3_SQ1;
////	ADC1->SQR3 |= 9; //VREF_SENSE
////
////	while (1) {
////		ADC1->CR2 |= ADC_CR2_SWSTART;
////		while (!(ADC1->SR & ADC_SR_EOC)) {}
////		bat = (uint32_t)ADC1->DR*vref_mv*3/(2*4095);
////	}
//}

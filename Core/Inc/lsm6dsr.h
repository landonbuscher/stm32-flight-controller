#ifndef INC_LSM6DSR_H_
#define INC_LSM6DSR_H_

#include "spi.h"

void lsm6dsr_init(void);
void lsm6dsr_config(void);

void lsm6dsr_spi_select(void);
void lsm6dsr_spi_deselect(void);

void lsm6dsr_get_gyro(int16_t*);
void lsm6dsr_get_xl(int16_t*);
void lsm6dsr_get_gyro_xl(int16_t*);
void lsm6dsr_get_gyro_phys(float*);
void lsm6dsr_get_xl_phys(float*);
void lsm6dsr_get_gyro_xl_phys(float*);

void lsm6dsr_calibration(void);

#endif /* INC_LSM6DSR_H_ */

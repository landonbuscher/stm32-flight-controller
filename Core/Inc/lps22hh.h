#ifndef INC_LPS22HH_H_
#define INC_LPS22HH_H_

#include "spi.h"

void lps22hh_init(void);
void lps22hh_config(void);

void lps22hh_spi_select(void);
void lps22hh_spi_deselect(void);

#endif /* INC_LPS22HH_H_ */

#ifndef INC_SPI_H_
#define INC_SPI_H_

#include <stdint.h>

typedef struct {
	void (*select)(void);
	void (*deselect)(void);
} Peripheral;

void spi_init(void);
uint8_t spi_read(Peripheral*, uint8_t);
void spi_read_burst(Peripheral*, uint8_t, uint8_t, uint8_t*);
void spi_write(Peripheral*, uint8_t, uint8_t);

#endif /* INC_SPI_H_ */

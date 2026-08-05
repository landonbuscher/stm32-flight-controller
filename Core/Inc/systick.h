#ifndef INC_SYSTICK_H_
#define INC_SYSTICK_H_

#include <stdint.h>

void systick_increment(void);
uint32_t systick_get_ms(void);

#endif /* INC_SYSTICK_H_ */

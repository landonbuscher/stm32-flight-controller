#ifndef INC_USART_H_
#define INC_USART_H_

#include <stdint.h>
#include <stdio.h>

void usart_init(void);
void usart_send(uint8_t);
void usart_print(char*);

#endif /* INC_USART_H_ */

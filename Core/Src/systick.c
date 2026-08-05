#include "systick.h"

//GLOBAL VARIABLES
static volatile uint32_t count_ms = 0;

//Called every tick (1ms) by SysTick_Handler
void systick_increment(void) {
	count_ms++;
}

//Getter function
uint32_t systick_get_ms(void) {
	return count_ms;
}

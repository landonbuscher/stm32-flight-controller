#ifndef INC_PID_H_
#define INC_PID_H_

#include <stdint.h>
#include "motor.h"

typedef struct {
	float Kp;
	float Ki;
	float Kd;
	float throttle;
	float pitch;
	float roll;
	float yaw;
	float pitch_setpoint;
	float roll_setpoint;
	float error_pitch;
	float error_roll;
	float pitch_prev;
	float roll_prev;
	float error_pitch_prev;
	float error_roll_prev;
	float error_pitch_cum;
	float error_roll_cum;
	float throttle_factor;
	float pitch_factor;
	float roll_factor;
	float axis_scale;
	float high;
	float low;
	uint8_t saturated;
} pid_data_t;

void pid_handler(float, float);
void pid_arm(void);
void pid_disarm(void);
uint8_t pid_get_armed(void);
void pid_set_throttle(float);
void pid_set_pitch_setpoint(float);
void pid_set_roll_setpoint(float);

#endif /* INC_PID_H_ */

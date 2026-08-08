#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_

typedef enum {
	FL, FR, RR, RL
} motor_position_t;

void motor_init(void);
void motor_global_output(float);
void motor_output(motor_position_t, float);

#endif /* INC_MOTOR_H_ */

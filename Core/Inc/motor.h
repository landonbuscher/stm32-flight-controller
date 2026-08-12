#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_

typedef enum {
	FL, FR, RR, RL
} motor_position_t;

typedef struct {
	motor_position_t position;
	volatile float output;
} motor_t;

void motor_init(void);
void motor_global_output(float);
void motor_output(motor_position_t, float);
motor_t* motor_get(motor_position_t);


#endif /* INC_MOTOR_H_ */

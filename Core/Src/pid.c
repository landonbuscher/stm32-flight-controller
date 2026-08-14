#include "pid.h"
#include "usart.h"
#include "timer.h"

static volatile uint8_t armed = 0;

static pid_data_t pid = {
		//Configurable variables
		.Kp = 0.5f,
		.Ki = 0.0f,
		.Kd = 0.02f,

		//Software variables
		.throttle = 20.0f,
		.pitch = 0.0f,
		.roll = 0.0f,
		.pitch_setpoint = 0.0f,
		.roll_setpoint = 0.0f,
		.error_pitch = 0.0f,
		.error_roll = 0.0f,
		.pitch_prev = 0.0f,
		.roll_prev = 0.0f,
		.error_pitch_prev = 0.0f,
		.error_roll_prev = 0.0f,
		.error_pitch_cum = 0.0f,
		.error_roll_cum = 0.0f,
		.throttle_factor = 0.0f,
		.pitch_factor = 0.0f,
		.roll_factor = 0.0f,
		.axis_scale = 1.0f,
		.high = 0.0f,
		.low = 0.0f,
		.saturated = 0
};

static void pid_correct(void);
static void pid_step(float, float);
static void pid_mix(void);

static void pid_correct() {
	float correction = 0.0f;
	if (pid.high>100.0f && pid.low<10.0f) {
		pid.saturated = 1;
		pid.axis_scale = 90.0f/(pid.high-pid.low);
		pid_mix();
	} else {
		pid.saturated = 0;
	}
	if (pid.high>100.0f && pid.low>=10.0f) {
		correction = 100.0f - pid.high;
	} else if (pid.high<=100.0f && pid.low<10.0f) {
		correction = 10.0f - pid.low;
	}
	for (motor_position_t motor_position=0; motor_position<4; motor_position++) {
		motor_t* motor = motor_get(motor_position);
		motor->output += correction;
		motor_output(motor->position, motor->output);
	}
}

void pid_handler(float pitch, float roll) {
	pid.axis_scale = 1.0f;
	pid_step(pitch, roll);
	pid_mix();
	pid_correct();
}

static void pid_step(float pitch, float roll) {
	pid.pitch_prev = pid.pitch;
	pid.roll_prev = pid.roll;
	pid.error_pitch_prev = pid.error_pitch;
	pid.error_roll_prev = pid.error_roll;
	pid.pitch = pitch;
	pid.roll = roll;
	pid.error_pitch = pid.pitch_setpoint - pitch;
	pid.error_roll = pid.roll_setpoint - roll;

	if ((pid.Ki * (pid.error_pitch_cum * DT) < 20 && pid.Ki * (pid.error_pitch_cum * DT) > -20) && !pid.saturated) {
		pid.error_pitch_cum += pid.error_pitch;
	}
	if ((pid.Ki * (pid.error_roll_cum * DT) < 20 && pid.Ki * (pid.error_roll_cum * DT) > -20) && !pid.saturated) {
		pid.error_roll_cum += pid.error_roll;
	}

	pid.throttle_factor = pid.throttle;
	pid.pitch_factor =
			(pid.Kp * pid.error_pitch) +
			(pid.Ki * (pid.error_pitch_cum * DT)) -
			(pid.Kd * (pid.pitch - pid.pitch_prev)/DT);
	pid.roll_factor =
			(pid.Kp * pid.error_roll) +
			(pid.Ki * (pid.error_roll_cum * DT)) -
			(pid.Kd * (pid.roll - pid.roll_prev)/DT);
}

static void pid_mix() {
	pid.high = -500.0f;
	pid.low = 500.0f;
	for (motor_position_t motor_position=0; motor_position<4; motor_position++) {
		motor_t* motor = motor_get(motor_position);
		float output=0.0f;
		switch (motor->position) {
		//pitch up = positive, roll right = positive
			case FL: output = pid.throttle_factor + (pid.pitch_factor + pid.roll_factor)*pid.axis_scale; break;
			case FR: output = pid.throttle_factor + (pid.pitch_factor - pid.roll_factor)*pid.axis_scale; break;
			case RR: output = pid.throttle_factor - (pid.pitch_factor + pid.roll_factor)*pid.axis_scale; break;
			case RL: output = pid.throttle_factor - (pid.pitch_factor - pid.roll_factor)*pid.axis_scale; break;
		}
		if (output>pid.high) {
			pid.high = output;
		}
		if (output<pid.low) {
			pid.low = output;
		}
		motor->output = output;
	}
}

void pid_arm(void) {
	if (!armed) {
		pid.error_pitch_cum = 0.0f;
		pid.error_roll_cum = 0.0f;
		armed = 1;
	}
}

void pid_disarm(void) {
	if (armed) {
		armed = 0;
		motor_global_output(0.0f);
	}
}

uint8_t pid_get_armed(void) {
	return armed;
}

void pid_set_throttle(float setpoint) {
	pid.throttle = setpoint;
}

void pid_set_pitch_setpoint(float setpoint) {
	pid.pitch_setpoint = setpoint;
}

void pid_set_roll_setpoint(float setpoint) {
	pid.roll_setpoint = setpoint;
}

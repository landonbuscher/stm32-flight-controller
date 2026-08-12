# STM32 Flight Controller - Bare-Metal Firmware
Attitude estimation and motor control firmware for a quadcopter flight controller, written exclusively by reference of data sheets and reference manuals with no HAL abstraction.

![Test bench setup including drone and flight controller](assets/board.jpg)

***

### Overview
This firmware reads a 6-axis IMU over SPI, fuses accelerometer and gyroscope data into a pitch and roll estimate using a complementary filter, and streams the result over USART for analysis. Every peripheral driver (clock tree, SPI, USART, timers, GPIO) was written from scratch using the respective documentation.

 **Current State:** *(Aug. 8 2026)* Attitude estimation and telemetry are working. A pitch/roll PID controller with motor mixing and output saturating handling is implemented but not yet flight-tested or tuned.
 
***

### Hardware
| Component | Part |
|:-|:-|
| Board | STEVAL-FCU001V2 |
| MCU | STM32F401CCU6 |
| IMU | LSM6DSR |
| Barometer | LPS22HH |

***

### Architecture
| Module | Responsibility |
|:-|:-|
| clock.c | Configure SYSCLK @ 84MHz using PLL from 16MHz HSI |
| spi.c | Configure SPI2; single-byte and burst reads, writes, chip-select |
| lsm6dsr.c | Configure IMU; raw->physical conversion; calibration |
| lps22hh.c | Configure barometer |
| timer.c | Configure TIM2 as 1kHz control loop |
| motor.c | Configure TIM4 CH1-4 as motor PWM outputs |
| usart.c | Configure USART; DMA-driven TX with ring-buffered message queue |
| systick.c | 1ms counter |
| pid.c | Pitch/roll PID, motor mixing, output saturation handling, arm/disarm |

***

### Design Decisions
* **Register-level drivers over HAL:** While HAL would've been faster to write, I chose to write all drivers at the register level to build my ability to comprehend and implement features by reference of technical documentation.
* **Complementary Filter:** Using a complementary filter drastically simplified the attitude estimation and is sufficient for a slow indoor flight around my desk, though a Kalman filter is a logical next step when I want to improve handling in more technical environments, implement the barometer, and better handle the sensor noise.
* **Preserving torque under saturation:** An edge case I discovered happened when one motor demanded more than 100% and another less than my minimum threshold of 10% power. Without correction, simply clamping the values to their high and low would change the power differential between motors, resulting in the attitude changing in unexpected ways. Instead, the mixer first tries to shift all four motors to ensure they are within the threshold. When the requested span exceeds the 10% to 100% threshold, the pitch and roll differentials are scaled by the same factor and shifted so that the attitude is controlled as intended. This is at the cost of altitude.

***

### Results
* **Measured control loop rate:** While TIM2 is configured to run at 1000Hz, using a logic analyzer showed the mean frequency to actually be 1005Hz, a 0.5% error, across trials. The measured value is what was used in the code to calculate dt to ensure this discrepency didn't propagate into the filter readings.
* **Gyroscope calibration:** When the drone first starts, it waits a set amount of time to settle to prevent movement like plugging in a battery from interfering, then collects data over a set time interval to remove the constant bias from gyroscope rate readings.
* **Accelerometer calibration:** The accelerometer does not recalibrate on startup as each axis was measured to have a small bias and sensitivity error relative to the datasheet. To obtain scale and offset calibration values, output values were recorded with the drone oriented on all 6 of its sides. By taking the average of the high and low readings, I could obtain the offset for each axis; taking half the difference between them gave the scale factor.

***

### Status & Roadmap
#### Working
* 84 MHz clock tree
* Bare-metal SPI driver shared between two sensors
* LSM6DSR configuration, calibration, unit conversion
* 1 kHz complementary filter producing pitch and roll
* 100 Hz USART telemetry over a DMA TX queue
* Motor output
#### Implemented, pending hardware validation
* Pitch/roll PID with motor mixing
* Output saturation handling and integral anti-windup
* Arm/disarm state machine
#### In progress
* Barometer integration for altitude readings
#### Next
* RC receiver input
* Kill switch, arm, disarm over RC receiver
* Gain tuning for PID coefficients
* Yaw axis implementation

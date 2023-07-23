#ifndef PWM_GPIO_H_
#define PWM_GPIO_H_
#include <zephyr/device.h>
#include <stdint.h>

void stop_pwm_pins(uint8_t *pin_gpios, uint8_t num_pins);
void start_pwm_pins(uint8_t *pin_gpios, uint8_t *freqs, uint8_t *duty_cycles, uint8_t num_pins);


#endif /* PWM_GPIO_H_ */
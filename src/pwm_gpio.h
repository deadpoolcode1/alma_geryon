#ifndef PWM_GPIO_H_
#define PWM_GPIO_H_
#include <zephyr/device.h>
#include <stdint.h>

enum {
    SIGNAL_CSET_1,
    SIGNAL_CSET_2,
    SIGNAL_PWM_1,
    SIGNAL_PWM_2,
    SIGNAL_NUM
};

void pwm_pins_init(void);
void stop_pwm_pins(uint8_t *pin_gpios, uint8_t num_pins);
void start_pwm_pins(uint8_t *pin_gpios, uint8_t *freqs, uint8_t *duty_cycles, uint8_t num_pins, int8_t index_shift);


#endif /* PWM_GPIO_H_ */
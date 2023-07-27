#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <stdbool.h>
#include "io.h"
#include "cset.h"


#define LOG_MODULE_NAME pwm_control
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_ERR);

#define MAX_NUM_PWM_PINS 4

void m_io_set(uint8_t pin_gpio, uint8_t value)
{
    if (pin_gpio == CSET_CH_1 || pin_gpio == CSET_CH_2 )
        cset_out(pin_gpio, 128 * value);
    else if (pin_gpio == IO_PWM1_OUT || pin_gpio == IO_PWM2_OUT)
        io_set(pin_gpio, value); 
}

struct pwm_pin_state {
    uint8_t pin_gpio;
    uint32_t period;
    uint32_t pulse_width;
    bool is_start;
    bool is_init;
    struct k_work_delayable work_timer_high; // Work for PWM pin high duration
    struct k_work_delayable work_timer_low;  // Work for PWM pin low duration
};

static struct pwm_pin_state pwm_pins[MAX_NUM_PWM_PINS];

static void work_high_callback(struct k_work *work)
{
    struct pwm_pin_state *pin_state = CONTAINER_OF(work, struct pwm_pin_state, work_timer_high);
    m_io_set(pin_state->pin_gpio, 0); // Set the PWM pin low
    k_work_schedule(&pin_state->work_timer_low, K_MSEC(pin_state->period - pin_state->pulse_width));
}

static void work_low_callback(struct k_work *work)
{
    struct pwm_pin_state *pin_state = CONTAINER_OF(work, struct pwm_pin_state, work_timer_low);
    m_io_set(pin_state->pin_gpio, 1); // Set the PWM pin high
    k_work_schedule(&pin_state->work_timer_high, K_MSEC(pin_state->pulse_width));
}

void start_pwm_pins(uint8_t *pin_gpios, uint8_t *freqs, uint8_t *duty_cycles, uint8_t num_pins, int8_t index_shift)
{
    if (num_pins > MAX_NUM_PWM_PINS) {
        return;
    }

    for (uint8_t i = 0; i < num_pins; i++) {
        uint8_t pin_gpio = pin_gpios[i];
        uint32_t freq = freqs[i];
        uint32_t duty_cycle = duty_cycles[i];

        uint32_t period_ms = 1000 / freq;
        uint32_t pulse_width_ms = (duty_cycle * period_ms) / 100;

        struct pwm_pin_state *pin_state = &pwm_pins[pin_gpio];
        pin_state->pin_gpio = pin_gpio;
        pin_state->period = period_ms;
        pin_state->pulse_width = pulse_width_ms;
        LOG_DBG("%d %d", period_ms, pulse_width_ms);
        pin_state->is_start = true;
        if (pin_state->is_init == false) {
            pin_state->is_init = true;
            k_work_init_delayable(&pin_state->work_timer_high, work_high_callback);
            k_work_init_delayable(&pin_state->work_timer_low, work_low_callback);
        }

        m_io_set(pin_state->pin_gpio, 0); // Set the PWM pin low initially
        if (i == index_shift) {
            k_work_schedule(&pin_state->work_timer_high, K_MSEC(pin_state->period / 2 - pin_state->pulse_width / 2));
        } else {
            k_work_schedule(&pin_state->work_timer_high, K_NO_WAIT);
        }
        LOG_DBG("Started PWM on pin %u with freq %u Hz and duty cycle %u%%", pin_gpio, freq, duty_cycle);
    }
}

void stop_pwm_pins(uint8_t *pin_gpios, uint8_t num_pins)
{
    if (num_pins > MAX_NUM_PWM_PINS) {
        return;
    }

    for (uint8_t i = 0; i < num_pins; i++) {
        uint8_t pin_gpio = pin_gpios[i];
        struct pwm_pin_state *pin_state = &pwm_pins[pin_gpio];
        if (pin_state->is_start) {
            pin_state->is_start = false;
            k_work_cancel_delayable(&pin_state->work_timer_high);
            k_work_cancel_delayable(&pin_state->work_timer_low);
        }

        m_io_set(pin_state->pin_gpio, 0); // Set the PWM pin low

        LOG_DBG("Stopped PWM on pin %u", pin_gpio);
    }
}

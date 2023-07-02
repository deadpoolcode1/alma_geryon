#include <zephyr/device.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/drivers/pwm.h> 
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>
#include <zephyr/random/rand32.h>
LOG_MODULE_REGISTER(main);

static const struct pwm_dt_spec sync_ch1_dev =
    PWM_DT_SPEC_GET(DT_ALIAS(sync_1));
static const struct pwm_dt_spec sync_ch2_dev =
    PWM_DT_SPEC_GET(DT_ALIAS(sync_2));
static const struct pwm_dt_spec sync_ch3_dev =
    PWM_DT_SPEC_GET(DT_ALIAS(sync_3));

#define MIN_PERIOD PWM_KHZ(250)
#define MAX_PERIOD PWM_SEC(1U)

struct pwm_config {
  uint32_t period;
  uint32_t pulse;
};

struct pwm_config sync_1_config = {.period = MIN_PERIOD, .pulse = 0};
struct pwm_config sync_2_config = {.period = MIN_PERIOD, .pulse = 0};

void timer_sync_1_work(struct k_timer *timer);
void timer_sync_2_work(struct k_timer *timer);
void sync_1_work(struct k_work *work);
void sync_2_work(struct k_work *work);
K_TIMER_DEFINE(timer_sync_1, timer_sync_1_work, NULL);
K_TIMER_DEFINE(timer_sync_2, timer_sync_2_work, NULL);
K_WORK_DEFINE(sync_1, sync_1_work);
K_WORK_DEFINE(sync_2, sync_2_work);

int max_shift = 360;
int current_freq = 250000;
int current_pulse = 50;
int current_shift = 120;
int current_jitter = 0;
uint32_t freq = 0;
uint32_t pulse = 0;
void sync_1_work(struct k_work *work) {
  
}

void sync_2_work(struct k_work *work) {
  
}

void timer_sync_1_work(struct k_timer *timer) {
  pwm_set_dt(&sync_ch2_dev, freq, pulse);
}

void timer_sync_2_work(struct k_timer *timer) {
  pwm_set_dt(&sync_ch3_dev, freq, pulse);
}

void main(void) {
  if (!device_is_ready(sync_ch1_dev.dev)) {
    LOG_ERR("Error: PWM device %s is not ready", sync_ch1_dev.dev->name);
    return;
  }

  if (!device_is_ready(sync_ch2_dev.dev)) {
    LOG_ERR("Error: PWM device %s is not ready", sync_ch2_dev.dev->name);
    return;
  }

  if (!device_is_ready(sync_ch3_dev.dev)) {
    LOG_ERR("Error: PWM device %s is not ready", sync_ch3_dev.dev->name);
    return;
  }

  while (1) {
    k_sleep(K_SECONDS(1U));
  }
}

#include <zephyr/shell/shell.h>
static int cmd_set_freq(const struct shell *shell, size_t argc, char **argv)
{
  uint32_t _freq = atoi(argv[1]);
  current_freq = _freq;
  freq = PWM_HZ(current_freq);
  pulse = current_pulse * freq / 100;
  uint32_t shift = 1E9 * current_shift / (current_freq * max_shift);
  LOG_INF("Freq %d - Pulse %d - Shift %d",  current_freq, current_pulse, shift);
  k_timer_start(&timer_sync_1, K_USEC(shift), K_NO_WAIT);
  k_timer_start(&timer_sync_2, K_USEC(shift * 2), K_NO_WAIT);
  pwm_set_dt(&sync_ch1_dev, freq, pulse);
	return 0;
}

static int cmd_get_shift(const struct shell *shell, size_t argc, char **argv)
{
  uint32_t _shift = atoi(argv[1]);
  current_shift = _shift;
  freq = PWM_HZ(current_freq);
  pulse = current_pulse * freq / 100;
  uint32_t shift = 1E9 * current_shift / (current_freq * max_shift);
  LOG_INF("Freq %d - Pulse %d - Shift %d",  current_freq, current_pulse, shift);
  k_timer_start(&timer_sync_1, K_USEC(shift), K_NO_WAIT);
  k_timer_start(&timer_sync_2, K_USEC(shift * 2), K_NO_WAIT);
  pwm_set_dt(&sync_ch1_dev, freq, pulse);
	return 0;
}

static int cmd_get_pulse(const struct shell *shell, size_t argc, char **argv)
{
  uint32_t _pulse = atoi(argv[1]);
  current_pulse = _pulse;
  freq = PWM_HZ(current_freq);
  pulse = current_pulse * freq / 100;
  uint32_t shift = 1E9 * current_shift / (current_freq * max_shift);
  LOG_INF("Freq %d - Pulse %d - Shift %d",  current_freq, current_pulse, shift);
  k_timer_start(&timer_sync_1, K_USEC(shift), K_NO_WAIT);
  k_timer_start(&timer_sync_2, K_USEC(shift * 2), K_NO_WAIT);
  pwm_set_dt(&sync_ch1_dev, freq, pulse);
	return 0;
}

static int cmd_get_jitter(const struct shell *shell, size_t argc, char **argv)
{
  int _jitter = atoi(argv[1]);
  if (_jitter >= 10 || _jitter <= -10) {
    shell_error(shell, "Invalid jitter input");
    return 0;
  }

  current_jitter = _jitter;
  int jitter = sys_rand32_get() % current_jitter;
  current_freq = current_freq + (current_freq * jitter / 100);
  freq = PWM_HZ(current_freq);
  pulse = current_pulse * freq / 100;
  uint32_t shift = 1E9 * current_shift / (current_freq * max_shift);
  LOG_INF("Freq %d - Pulse %d - Shift %d - Jitter %d (%d)",  current_freq, current_pulse, shift, jitter, current_jitter);
  k_timer_start(&timer_sync_1, K_USEC(shift), K_NO_WAIT);
  k_timer_start(&timer_sync_2, K_USEC(shift * 2), K_NO_WAIT);
  pwm_set_dt(&sync_ch1_dev, freq, pulse);
	return 0;
}

static int cmd_set_start(const struct shell *shell, size_t argc, char **argv)
{
  freq = PWM_HZ(current_freq);
  pulse = current_pulse * freq / 100;
  uint32_t shift = 1E9 * current_shift / (current_freq * max_shift);
  LOG_INF("Freq %d - Pulse %d - Shift %d",  current_freq, current_pulse, shift);
  k_timer_start(&timer_sync_1, K_USEC(shift), K_NO_WAIT);
  k_timer_start(&timer_sync_2, K_USEC(shift * 2), K_NO_WAIT);
  pwm_set_dt(&sync_ch1_dev, freq, pulse);
	return 0;
}

static int cmd_get_stop(const struct shell *shell, size_t argc, char **argv)
{
  pwm_set_dt(&sync_ch1_dev, current_freq, 0);
  pwm_set_dt(&sync_ch2_dev, current_freq, 0);
  pwm_set_dt(&sync_ch3_dev, current_freq, 0);
	return 0;
}

static int cmd_get_config(const struct shell *shell, size_t argc, char **argv)
{
  shell_print(shell, "Frequency %d - Pulse %d - Shift %d", current_freq, 50, current_shift);
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_pwm,
	SHELL_CMD(config, NULL, "Get current configuration", cmd_get_config),
	SHELL_CMD(freq, NULL, "Set the frequency", cmd_set_freq),
	SHELL_CMD(shift, NULL, "Set the shift degree", cmd_get_shift),
  SHELL_CMD(pulse, NULL, "Set the pulse", cmd_get_pulse),
  SHELL_CMD(jitter, NULL, "Set the jitter", cmd_get_jitter),
	SHELL_SUBCMD_SET_END);

SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_test,
	SHELL_CMD(start, NULL, "Start generating the PWM pulse ", cmd_set_start),
	SHELL_CMD(stop, NULL, "Stop generating the PWM pulse", cmd_get_stop),
	SHELL_SUBCMD_SET_END);


SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_cmd,
	SHELL_CMD(pwm, &sub_pwm, "Set/Get for PWM", NULL),
	SHELL_CMD(test, &sub_test, "Test command", NULL),
	SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(kama, &sub_cmd, "KamaCode shell interface", NULL);

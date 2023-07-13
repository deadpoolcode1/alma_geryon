#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(geryon_main);

#include "protocol.h"

extern void pwm_init(void);
void main(void)
{
  pwm_init();
  protocol_init();
  while(1) {
    k_sleep(K_SECONDS(1));
  }
}

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>

LOG_MODULE_REGISTER(geryon_main);

#include "app_event_manager.h"
#include "cmon.h"
#include "cset.h"
#include "io.h"
#include "main.h"
#include "mcp466.h"
#include "protocol.h"
#include "pwm.h"

struct alma_data global_alma_data = {.sync_freq = 250000,
                                     .jitter = 1,
                                     .pulse_freq = 100,
                                     .pulse_dc_1 = 5,
                                     .pulse_dc_2 = 5,
                                     .op_mode = 1,
                                     .pulse_mode = 1};

void main_init_flow(void) {
  io_set(IO_STATUS_OUT, 0);
  io_set(IO_PULSE1_OUT, 1);
  io_set(IO_PULSE2_OUT, 0);
  io_set(IO_DRV_EN1_OUT, 1);
  io_set(IO_DRV_EN2_OUT, 0);
  io_set(IO_PWM1_OUT, 0);
  io_set(IO_PWM2_OUT, 0);
  cset_out(CSET_CH_1, 0);
  cset_out(CSET_CH_2, 0);
  mcp466_write(MCP466_CH_1, 0, 0);
  mcp466_write(MCP466_CH_2, 0, 0);
  mcp466_write(MCP466_CH_1, 1, 0);
  mcp466_write(MCP466_CH_2, 1, 0);
  io_set(IO_RS232_SHDN_OUT, 1);
  io_set(IO_RS232_EN_OUT, 0);
  io_set(IO_RESET_N_OUT, 1);
  io_set(IO_STATE1_OUT, 1);
  io_set(IO_STATE2_OUT, 0);
  io_set(IO_STATE2_OUT, 0);
}

void main_close_laser_flow(void) {
  io_set(IO_PWM1_OUT, 0);
  io_set(IO_PWM2_OUT, 0);
  cset_out(CSET_CH_1, 0);
  cset_out(CSET_CH_2, 0);
  io_set(IO_PULSE1_OUT, 0);
  io_set(IO_PULSE2_OUT, 0);
  io_set(IO_DRV_EN1_OUT, 0);
  io_set(IO_DRV_EN2_OUT, 0);
  io_set(IO_STATUS_OUT, 1);
}

void main_opmode_1_flow(void) {
  io_set(IO_PULSE1_OUT, 1);
  io_set(IO_PULSE2_OUT, 0);
  io_set(IO_DRV_EN1_OUT, 1);
  io_set(IO_DRV_EN2_OUT, 0);
  if (global_alma_data.pulse_mode == 1) {
    cset_out(CSET_CH_1, 128);
    io_set(IO_PWM1_OUT, 1);
  } else if (global_alma_data.pulse_mode == 0) {
    cset_out(CSET_CH_1, 128);
    io_set(IO_PWM1_OUT, 1);
  }
  io_set(IO_STATE1_OUT, 0);
  io_set(IO_STATE2_OUT, 0);
  io_set(IO_STATE2_OUT, 0);
}

void main_opmode_2_flow(void) {
  io_set(IO_PULSE1_OUT, 0);
  io_set(IO_PULSE2_OUT, 1);
  io_set(IO_DRV_EN1_OUT, 0);
  io_set(IO_DRV_EN2_OUT, 1);
  if (global_alma_data.pulse_mode == 1) {
    cset_out(CSET_CH_2, 128);
    io_set(IO_PWM2_OUT, 1);
  } else if (global_alma_data.pulse_mode == 0) {
    cset_out(CSET_CH_2, 128);
    io_set(IO_PWM2_OUT, 1);
  }
  io_set(IO_STATE1_OUT, 0);
  io_set(IO_STATE2_OUT, 0);
  io_set(IO_STATE2_OUT, 0);
}

void main_opmode_3_flow(void) {
  io_set(IO_PULSE1_OUT, 1);
  io_set(IO_PULSE2_OUT, 1);
  io_set(IO_DRV_EN1_OUT, 1);
  io_set(IO_DRV_EN2_OUT, 1);
  if (global_alma_data.pulse_mode == 1) {
    cset_out(CSET_CH_1, 128);
    cset_out(CSET_CH_2, 128);
    io_set(IO_PWM1_OUT, 1);
    io_set(IO_PWM2_OUT, 1);
  } else if (global_alma_data.pulse_mode == 0) {
    cset_out(CSET_CH_1, 128);
    cset_out(CSET_CH_2, 128);
    io_set(IO_PWM1_OUT, 1);
    io_set(IO_PWM2_OUT, 1);
  }
  io_set(IO_STATE1_OUT, 0);
  io_set(IO_STATE2_OUT, 0);
  io_set(IO_STATE2_OUT, 1);
}

void main_opmode_4_flow(void) {
  io_set(IO_PULSE1_OUT, 1);
  io_set(IO_PULSE2_OUT, 1);
  io_set(IO_DRV_EN1_OUT, 1);
  io_set(IO_DRV_EN2_OUT, 1);
  if (global_alma_data.pulse_mode == 1) {
    cset_out(CSET_CH_1, 128);
    cset_out(CSET_CH_2, 128);
    io_set(IO_PWM1_OUT, 1);
    io_set(IO_PWM2_OUT, 1);
  } else if (global_alma_data.pulse_mode == 0) {
    cset_out(CSET_CH_1, 128);
    cset_out(CSET_CH_2, 128);
    io_set(IO_PWM1_OUT, 1);
    io_set(IO_PWM2_OUT, 1);
  }
  io_set(IO_STATE1_OUT, 1);
  io_set(IO_STATE2_OUT, 1);
  io_set(IO_STATE2_OUT, 0);
}

void main(void) {
  io_init();
  cset_init();
  cmon_init();
  pwm_init();
  mcp466_init();
  main_init_flow();
  protocol_init();
  while (1) {
    struct app_event evt;
    /* Wait for the next event */
    int err = app_event_manager_get(&evt);
    if (err) {
      LOG_WRN("Unable to get event. Err: %i", err);
      continue;
    }

    switch (evt.type) {
    case APP_EVENT_IN_GLOBAL_EN_RISING: {
      if (global_alma_data.pulse_mode == 1) {
        io_set(IO_PWM1_OUT, 1);
        io_set(IO_PWM2_OUT, 1);
      } else if (global_alma_data.pulse_mode == 2) {
        cset_out(CSET_CH_1, cset_get_last_value(CSET_CH_1));
        cset_out(CSET_CH_2, cset_get_last_value(CSET_CH_2));
      }
      /* Start pulse flow */
      pwm_start();
      io_set(IO_RS232_EN_OUT, 1); // Disable RS232
      break;
    }
    case APP_EVENT_IN_GLOBAL_EN_FALLING: {
      /* Stop pusle flow */
      pwm_stop();
      io_set(IO_RS232_EN_OUT, 0); // Enable RS232
      break;
    }
    case APP_EVENT_IN_RESET: {
      io_set(IO_RESET_N_OUT, 1);
      if (global_alma_data.op_mode == 1) {
	io_set(IO_PULSE1_OUT, 1);
	io_set(IO_PULSE2_OUT, 0);
	io_set(IO_DRV_EN1_OUT, 1);
	io_set(IO_DRV_EN2_OUT, 0);
	io_set(IO_STATE1_OUT, 0);
	io_set(IO_STATE2_OUT, 0);
	io_set(IO_STATE2_OUT, 0);
      } else if (global_alma_data.op_mode == 2) {
	io_set(IO_PULSE1_OUT, 0);
	io_set(IO_PULSE2_OUT, 1);
	io_set(IO_DRV_EN1_OUT, 0);
	io_set(IO_DRV_EN2_OUT, 1);
	io_set(IO_STATE1_OUT, 0);
	io_set(IO_STATE2_OUT, 0);
	io_set(IO_STATE2_OUT, 0);
      } else if (global_alma_data.op_mode == 3) {
	io_set(IO_PULSE1_OUT, 1);
	io_set(IO_PULSE2_OUT, 1);
	io_set(IO_DRV_EN1_OUT, 1);
	io_set(IO_DRV_EN2_OUT, 1);
	io_set(IO_STATE1_OUT, 0);
	io_set(IO_STATE2_OUT, 0);
	io_set(IO_STATE2_OUT, 1);
      } else if (global_alma_data.op_mode == 4) {
	io_set(IO_PULSE1_OUT, 1);
	io_set(IO_PULSE2_OUT, 1);
	io_set(IO_DRV_EN1_OUT, 1);
	io_set(IO_DRV_EN2_OUT, 1);
	io_set(IO_STATE1_OUT, 1);
	io_set(IO_STATE2_OUT, 1);
	io_set(IO_STATE2_OUT, 0);
      }
      break;
    }
    case APP_EVENT_IN_OT:
      main_close_laser_flow();
      protop_send_error_message(OT_ERR);
      break;
    case APP_EVENT_IN_OC2:
      main_close_laser_flow();
      protop_send_error_message(OC2_ERR);
      break;
    case APP_EVENT_IN_OV2:
      main_close_laser_flow();
      protop_send_error_message(OV2_ERR);
      break;
    case APP_EVENT_IN_FLT:
      main_close_laser_flow();
      protop_send_error_message(FLT_ERR);
      break;
    case APP_EVENT_IN_OV1:
      main_close_laser_flow();
      protop_send_error_message(OV1_ERR);
      break;
    case APP_EVENT_IN_OC1:
      main_close_laser_flow();
      protop_send_error_message(OC1_ERR);
      break;
    default:
      break;
    }
  }
}

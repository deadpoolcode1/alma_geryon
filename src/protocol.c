#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>

LOG_MODULE_REGISTER(geryon_uart, LOG_LEVEL_ERR);

#include "protocol.h"
#include "main.h"
#include "io.h"

extern struct alma_data global_alma_data;

static const struct device *g_proto_uart = NULL;
K_MSGQ_DEFINE(g_proto_uart_queue, 1, 4096, 4);

static struct proto_data g_proto_data;
static struct proto_data *p_proto_data = &g_proto_data;

static void proto_hw_rx_callback(const struct device *dev, void *user_data) {
  uint8_t c;

  if (!uart_irq_update(g_proto_uart)) {
    return;
  }

  if (uart_irq_rx_ready(g_proto_uart)) {
    uart_fifo_read(g_proto_uart, &c, 1);

    k_msgq_put(&g_proto_uart_queue, &c, K_NO_WAIT);
  }
}

static void proto_hw_init(void) {
  g_proto_uart = DEVICE_DT_GET(DT_ALIAS(alma_uart));
  if (!device_is_ready(g_proto_uart)) {
    LOG_ERR("UART device not found!");
    return;
  }
  uart_irq_callback_user_data_set(g_proto_uart, proto_hw_rx_callback, NULL);
  uart_irq_rx_enable(g_proto_uart);
}

static int proto_hw_send(uint8_t *data, int length) {
  for (int i = 0; i < length; i++) {
    uart_poll_out(g_proto_uart, data[i]);
  }
  return length;
}

static uint8_t protocol_calc_crc(uint8_t *buffer, uint8_t length) {
  uint16_t crc = 0;
  for (int i = 0; i < length; i++) {
    crc += buffer[i];
  }

  return (uint8_t)(crc & 0xFF);
}

static struct k_thread protocol_thread;
static k_tid_t thread_protocol_id;
#define PROTO_STACK_SIZE 4096
static K_KERNEL_STACK_DEFINE(app_stack, PROTO_STACK_SIZE);

#define PROTO_START_MSG (0x1B)
#define PROTO_END_MSG (0x0D)
#define PROTO_RECV_MAX_SIZE (128)
#define PROTO_CMD_OFFSET (5)
#define PROTO_SUB_CMD_OFFSET (6)
#define PROTO_SUB_CMD_CH_OFFSET (7)

typedef void (*proto_res_handler)(uint8_t *buffer, uint8_t len);

struct proto_req_cmd_info {
  uint8_t id;
  char cmd;
  char sub_cmd;
  proto_res_handler handler;
};

static void proto_req_cmd_set_freq_handler(uint8_t *buffer, uint8_t len);
static void proto_req_cmd_get_freq_handler(uint8_t *buffer, uint8_t len);
static void proto_req_cmd_set_duty_handler(uint8_t *buffer, uint8_t len);
static void proto_req_cmd_get_duty_handler(uint8_t *buffer, uint8_t len);
static void proto_req_cmd_set_current_handler(uint8_t *buffer, uint8_t len);
static void proto_req_cmd_get_current_handler(uint8_t *buffer, uint8_t len);
static void proto_req_cmd_set_operation_handler(uint8_t *buffer, uint8_t len);
static void proto_req_cmd_set_drv_pulse_mode_handler(uint8_t *buffer,
                                                     uint8_t len);
static void proto_req_cmd_set_sync_freq_handler(uint8_t *buffer, uint8_t len);
static void proto_req_cmd_set_jitter_handler(uint8_t *buffer, uint8_t len);
static void proto_req_cmd_set_fan_operation_handler(uint8_t *buffer,
                                                    uint8_t len);
static void proto_req_cmd_set_keep_alive_handler(uint8_t *buffer, uint8_t len);
static void proto_req_cmd_get_error_handler(uint8_t *buffer, uint8_t len);
static void proto_req_cmd_get_version_handler(uint8_t *buffer, uint8_t len);

struct proto_req_cmd_info proto_req_cmd_list[PROTO_REQ_CMD_MAX] = {
    {.id = PROTO_REQ_CMD_SET_FREQ,
     .cmd = 'S',
     .sub_cmd = 'F',
     .handler = proto_req_cmd_set_freq_handler},
    {.id = PROTO_REQ_CMD_GET_FREQ,
     .cmd = 'G',
     .sub_cmd = 'F',
     .handler = proto_req_cmd_get_freq_handler},
    {.id = PROTO_REQ_CMD_SET_DUTY,
     .cmd = 'S',
     .sub_cmd = 'D',
     .handler = proto_req_cmd_set_duty_handler},
    {.id = PROTO_REQ_CMD_GET_DUTY,
     .cmd = 'G',
     .sub_cmd = 'D',
     .handler = proto_req_cmd_get_duty_handler},
    {.id = PROTO_REQ_CMD_SET_CURRENT,
     .cmd = 'S',
     .sub_cmd = 'C',
     .handler = proto_req_cmd_set_current_handler},
    {.id = PROTO_REQ_CMD_GET_CURRENT,
     .cmd = 'G',
     .sub_cmd = 'C',
     .handler = proto_req_cmd_get_current_handler},
    {.id = PROTO_REQ_CMD_SET_OPERATION,
     .cmd = 'S',
     .sub_cmd = 'M',
     .handler = proto_req_cmd_set_operation_handler},
    {.id = PROTO_REQ_CMD_SET_DRV_PULSE_MODE,
     .cmd = 'S',
     .sub_cmd = 'P',
     .handler = proto_req_cmd_set_drv_pulse_mode_handler},
    {.id = PROTO_REQ_CMD_SET_SYNC_FREQ,
     .cmd = 'S',
     .sub_cmd = 'S',
     .handler = proto_req_cmd_set_sync_freq_handler},
    {.id = PROTO_REQ_CMD_SET_JITTER,
     .cmd = 'S',
     .sub_cmd = 'J',
     .handler = proto_req_cmd_set_jitter_handler},
    {.id = PROTO_REQ_CMD_SET_FAN_OPERATION,
     .cmd = 'C',
     .sub_cmd = 'F',
     .handler = proto_req_cmd_set_fan_operation_handler},
    {.id = PROTO_REQ_CMD_SET_KEEP_ALIVE,
     .cmd = 'C',
     .sub_cmd = 'K',
     .handler = proto_req_cmd_set_keep_alive_handler},
    {.id = PROTO_REQ_CMD_GET_ERROR,
     .cmd = 'E',
     .sub_cmd = 'E',
     .handler = proto_req_cmd_get_error_handler},
    {.id = PROTO_REQ_CMD_GET_VERSION,
     .cmd = 'C',
     .sub_cmd = 'V',
     .handler = proto_req_cmd_get_version_handler},
};

enum protocol_stage_e { PROTO_STAGE_WAIT_START, PROTO_STAGE_RECEVING };

enum protocol_status_e {
  PROTO_STATUS_IN_PROCESS = 0,
  PROTO_STATUS_ERROR = 1,
  PROTO_STATUS_READY = 2,
};

struct protocol_info {
  enum protocol_stage_e parseState;
  uint8_t *pbuf;
  size_t buf_size;
  size_t stream_idx;
};

static struct protocol_info g_proto_info;

static void protocol_info_init(char *buf, int len) {
  g_proto_info.parseState = PROTO_STATUS_IN_PROCESS;
  g_proto_info.pbuf = buf;
  g_proto_info.buf_size = len;
  g_proto_info.stream_idx = 0;
}

enum protocol_status_e protocol_accum_frame(struct protocol_info *pInfo,
                                            const uint8_t ch) {
  if (pInfo->stream_idx >= (pInfo->buf_size - 1)) {
    pInfo->stream_idx = 0;
    pInfo->parseState = PROTO_STAGE_WAIT_START;
  }

  switch (pInfo->parseState) {
  case PROTO_STAGE_WAIT_START:
    pInfo->stream_idx = 0;
    if (PROTO_START_MSG == ch) {
      pInfo->pbuf[pInfo->stream_idx++] = ch;
      pInfo->parseState = PROTO_STAGE_RECEVING;
    } else {
      pInfo->parseState = PROTO_STAGE_WAIT_START;
    }
    break;
  case PROTO_STAGE_RECEVING:
    pInfo->pbuf[pInfo->stream_idx++] = ch;
    if (PROTO_END_MSG == ch) {
      pInfo->pbuf[pInfo->stream_idx] = '\0';
      pInfo->parseState = PROTO_STAGE_WAIT_START;
      return PROTO_STATUS_READY;
    }
    break;
  }
  return PROTO_STATUS_IN_PROCESS;
}

static const uint8_t *protocol_accum_buf(struct protocol_info *pInfo) {
  return pInfo->pbuf;
}

static uint16_t protocol_accum_len(struct protocol_info *pInfo) {
  return (uint16_t)pInfo->stream_idx;
}

static char procol_hex_ascii(char hex) {
  if (hex >= '0' && hex <= '9') {
    return hex - '0';
  } else if (hex >= 'A' && hex <= 'F') {
    return hex - 'A' + 10;
  } else if (hex >= 'a' && hex <= 'f') {
    return hex - 'a' + 10;
  }
  return -1; // Invalid hex character
}

static void protocol_request_parser(void) {
  const uint8_t *pbuf = protocol_accum_buf(&g_proto_info);
  const uint16_t len = protocol_accum_len(&g_proto_info);
  LOG_HEXDUMP_INF(pbuf, len, "REQUEST");
  uint8_t crc = protocol_calc_crc((uint8_t *)pbuf, len - 3);

  uint8_t recv_crc =
      procol_hex_ascii(pbuf[len - 3]) << 4 | procol_hex_ascii(pbuf[len - 2]);
  if (recv_crc != crc) {
    LOG_DBG("Invalid CRC frame 0x%02x - 0x%02x", crc, recv_crc);
    return;
  }

  uint8_t cmd = pbuf[PROTO_CMD_OFFSET];
  uint8_t sub_cmd = pbuf[PROTO_SUB_CMD_OFFSET];
  LOG_DBG("0x%02x - 0x%02x", cmd, sub_cmd);
  for (int i = 0; i < PROTO_REQ_CMD_MAX; i++) {
    if (cmd == proto_req_cmd_list[i].cmd &&
        sub_cmd == proto_req_cmd_list[i].sub_cmd) {
      if (proto_req_cmd_list[i].handler != NULL) {
        proto_req_cmd_list[i].handler((uint8_t *)pbuf, len);
      }
    }
  }
}

void protocol_thread_handler(void) {
  uint8_t recv_buff[PROTO_RECV_MAX_SIZE] = {0x00};
  char c;
  protocol_info_init(recv_buff, PROTO_RECV_MAX_SIZE);
  while (1) {
    k_msgq_get(&g_proto_uart_queue, &c, K_FOREVER);
    const enum protocol_status_e wait_status =
        protocol_accum_frame(&g_proto_info, c);
    if (PROTO_STATUS_IN_PROCESS != wait_status) {
      if (PROTO_STATUS_READY == wait_status) {
        protocol_request_parser();
      }

      protocol_info_init(recv_buff, PROTO_RECV_MAX_SIZE);
    }
  }
}

void protocol_init(void) {
  proto_hw_init();

	p_proto_data->sync_freq = global_alma_data.sync_freq;
  p_proto_data->jitter_value = global_alma_data.jitter;
  p_proto_data->operation = global_alma_data.op_mode;
  p_proto_data->drive_pulse_mode = global_alma_data.pulse_mode;

  thread_protocol_id = k_thread_create(
      &protocol_thread, app_stack, K_KERNEL_STACK_SIZEOF(app_stack),
      (k_thread_entry_t)protocol_thread_handler, NULL, NULL, NULL,
      K_LOWEST_APPLICATION_THREAD_PRIO, 0, K_NO_WAIT);
}

static void proto_req_cmd_set_freq_handler(uint8_t *buffer, uint8_t len) {
  LOG_DBG("Set frequency");
  uint8_t freq = buffer[7] - '0';
  if (freq < 1 || freq > 10) {
    LOG_DBG("Frequency is wrong");
    return;
  }

  p_proto_data->freq = freq;
  global_alma_data.pulse_freq = freq;
  LOG_DBG("Frequency is set %d", freq);
}

static void proto_req_cmd_get_freq_handler(uint8_t *buffer, uint8_t len) {
  LOG_DBG("Get frequency");
  uint8_t res_buf[128] = {0x00};
  uint8_t offset = 0;
  res_buf[offset++] = 0x1B;
  res_buf[offset++] = '0';
  res_buf[offset++] = '0';
  res_buf[offset++] = '0';
  res_buf[offset++] = '1';
  res_buf[offset++] = 'G';
  res_buf[offset++] = 'F';
  res_buf[offset++] = global_alma_data.pulse_freq + '0';
  uint8_t crc = protocol_calc_crc(res_buf, offset);
  res_buf[offset++] = (crc / 10) + '0';
  res_buf[offset++] = (crc % 10) + '0';
  res_buf[offset++] = 0x0D;
  LOG_HEXDUMP_INF(res_buf, offset, "GET_FREQ");
  proto_hw_send(res_buf, offset);
}

static void proto_req_cmd_set_duty_handler(uint8_t *buffer, uint8_t len) {
  uint8_t duty = procol_hex_ascii(buffer[8]) * 10 + procol_hex_ascii(buffer[9]);
  uint8_t channel = buffer[PROTO_SUB_CMD_CH_OFFSET] - '0';
  if (duty < 9 || duty > 30) {
    LOG_ERR("Duty is wrong %d", duty);
    return;
  }

  if (channel > SYNC_MAX_CH) {
    LOG_DBG("Channel is wrong %d", channel);
    return;
  }

  p_proto_data->duty[channel] = duty;
  global_alma_data.pulse_dc[channel] = duty;
  LOG_DBG("Duty channel %d is set %d", channel, duty);
}

static void proto_req_cmd_get_duty_handler(uint8_t *buffer, uint8_t len) {
  uint8_t res_buf[128] = {0x00};
  uint8_t offset = 0;
  uint8_t channel = buffer[PROTO_SUB_CMD_CH_OFFSET] - '0';
  uint8_t duty =  global_alma_data.pulse_dc[channel];
  res_buf[offset++] = 0x1B;
  res_buf[offset++] = '0';
  res_buf[offset++] = '0';
  res_buf[offset++] = '0';
  res_buf[offset++] = '2';
  res_buf[offset++] = 'G';
  res_buf[offset++] = 'D';
  res_buf[offset++] = channel + '0';
  res_buf[offset++] = (duty / 10) + '0';
  res_buf[offset++] = (duty % 10) + '0';
  uint8_t crc = protocol_calc_crc(res_buf, offset);
  res_buf[offset++] = (crc / 10) + '0';
  res_buf[offset++] = (crc % 10) + '0';
  res_buf[offset++] = 0x0D;
  proto_hw_send(res_buf, offset);
}

static void proto_req_cmd_set_current_handler(uint8_t *buffer, uint8_t len) {
  uint8_t current =
      procol_hex_ascii(buffer[8]) * 10 + procol_hex_ascii(buffer[9]);
  uint8_t channel = buffer[PROTO_SUB_CMD_CH_OFFSET] - '0';
  if (current < 10 || current > 90) {
    LOG_DBG("Current is wrong %d", current);
    return;
  }

  if (channel > SYNC_MAX_CH) {
    LOG_DBG("Channel is wrong %d", channel);
    return;
  }

  p_proto_data->current[channel] = current;
  LOG_DBG("Current channel %d is set %d", channel, current);
}

static void proto_req_cmd_get_current_handler(uint8_t *buffer, uint8_t len) {
  uint8_t res_buf[128] = {0x00};
  uint8_t offset = 0;
  uint8_t channel = buffer[PROTO_SUB_CMD_CH_OFFSET] - '0';
  uint8_t current = p_proto_data->current[channel];
  res_buf[offset++] = 0x1B;
  res_buf[offset++] = '0';
  res_buf[offset++] = '0';
  res_buf[offset++] = '0';
  res_buf[offset++] = '2';
  res_buf[offset++] = 'G';
  res_buf[offset++] = 'C';
  res_buf[offset++] = channel + '0';
  res_buf[offset++] = (current / 10) + '0';
  res_buf[offset++] = (current % 10) + '0';
  uint8_t crc = protocol_calc_crc(res_buf, offset);
  res_buf[offset++] = (crc / 10) + '0';
  res_buf[offset++] = (crc % 10) + '0';
  res_buf[offset++] = 0x0D;
  proto_hw_send(res_buf, offset);
}

static void proto_req_cmd_set_operation_handler(uint8_t *buffer, uint8_t len) {
  uint8_t operation = buffer[7] - '0';
  if (operation > 4) {
    LOG_DBG("Operation is wrong %d", operation);
    return;
  }

  p_proto_data->operation = operation;
  global_alma_data.op_mode = operation;
  LOG_DBG("Operation is set %d", operation);
}

static void proto_req_cmd_set_drv_pulse_mode_handler(uint8_t *buffer,
                                                     uint8_t len) {
  uint8_t drv_pulse = buffer[7] - '0';
  if (drv_pulse > 2) {
    LOG_DBG("Driver Pulse is wrong %d", drv_pulse);
    return;
  }

  p_proto_data->drive_pulse_mode = drv_pulse;
  global_alma_data.pulse_mode = drv_pulse;
  LOG_DBG("Driver Pulse is set %d", drv_pulse);
}

static void proto_req_cmd_set_sync_freq_handler(uint8_t *buffer, uint8_t len) {
  uint16_t sync_freq = procol_hex_ascii(buffer[7]) * 100 +
                       procol_hex_ascii(buffer[8]) * 10 +
                       procol_hex_ascii(buffer[9]);
  if (sync_freq > 300 || sync_freq < 250) {
    LOG_DBG("Sync Freq is wrong %d", sync_freq);
    return;
  }

  p_proto_data->sync_freq = sync_freq;
  global_alma_data.sync_freq = sync_freq;
  LOG_DBG("Sync Frequency is set %d", sync_freq);
}

static void proto_req_cmd_set_jitter_handler(uint8_t *buffer, uint8_t len) {
  uint8_t jitter = (buffer[7] - '0');
  if (jitter > 5) {
    LOG_DBG("Jitter is wrong %d", jitter);
    return;
  }

  p_proto_data->jitter_value = jitter;
  global_alma_data.jitter = jitter;
  LOG_DBG("Jitter is set %d", jitter);
}

static void proto_req_cmd_set_fan_operation_handler(uint8_t *buffer,
                                                    uint8_t len) {
  uint8_t fan = (buffer[7] - '0');
  if (fan != 0 && fan != 1) {
    LOG_DBG("Fan operation is wrong %d", fan);
    return;
  }

  p_proto_data->fan_operation = fan;
  if (fan)
	io_set(IO_FAN_OUT, 1);
  else
	io_set(IO_FAN_OUT, 0);
  LOG_DBG("Fan operation is set %d", fan);
}

static void proto_req_cmd_set_keep_alive_handler(uint8_t *buffer, uint8_t len) {
  uint8_t res_buf[128] = {0x00};
  uint8_t offset = 0;
  res_buf[offset++] = 0x1B;
  res_buf[offset++] = '0';
  res_buf[offset++] = '0';
  res_buf[offset++] = '0';
  res_buf[offset++] = '1';
  res_buf[offset++] = 'C';
  res_buf[offset++] = 'K';
  res_buf[offset++] = 1 + '0';
  uint8_t crc = protocol_calc_crc(res_buf, offset);
  res_buf[offset++] = (crc / 10) + '0';
  res_buf[offset++] = (crc % 10) + '0';
  res_buf[offset++] = 0x0D;
  proto_hw_send(res_buf, offset);
}

static void proto_req_cmd_get_error_handler(uint8_t *buffer, uint8_t len) {
  uint8_t res_buf[128] = {0x00};
  uint8_t offset = 0;
  res_buf[offset++] = 0x1B;
  res_buf[offset++] = '0';
  res_buf[offset++] = '0';
  res_buf[offset++] = '0';
  res_buf[offset++] = '1';
  res_buf[offset++] = 'E';
  res_buf[offset++] = 'E';
  res_buf[offset++] = p_proto_data->error + '0';
  uint8_t crc = protocol_calc_crc(res_buf, offset);
  res_buf[offset++] = (crc / 10) + '0';
  res_buf[offset++] = (crc % 10) + '0';
  res_buf[offset++] = 0x0D;
  proto_hw_send(res_buf, offset);
}

static void proto_req_cmd_get_version_handler(uint8_t *buffer, uint8_t len) {
  uint8_t res_buf[128] = {0x00};
  uint8_t offset = 0;
  res_buf[offset++] = 0x1B;
  res_buf[offset++] = '0';
  res_buf[offset++] = '0';
  res_buf[offset++] = '0';
  res_buf[offset++] = '1';
  res_buf[offset++] = 'C';
  res_buf[offset++] = 'V';
  int ver_len = sprintf(&res_buf[offset], "%s", "Geryon_20230305_01");
  offset += ver_len;
  res_buf[3] = (ver_len / 10) + '0';
  res_buf[4] = (ver_len % 10) + '0';
  uint8_t crc = protocol_calc_crc(res_buf, offset);
  res_buf[offset++] = (crc / 10) + '0';
  res_buf[offset++] = (crc % 10) + '0';
  res_buf[offset++] = 0x0D;
  proto_hw_send(res_buf, offset);
}

void protop_send_error_message(uint8_t error) {
  uint8_t res_buf[128] = {0x00};
  uint8_t offset = 0;
  res_buf[offset++] = 0x1B;
  res_buf[offset++] = '0';
  res_buf[offset++] = '0';
  res_buf[offset++] = '0';
  res_buf[offset++] = '1';
  res_buf[offset++] = 'E';
  res_buf[offset++] = 'E';
  res_buf[offset++] = error + '0';
  uint8_t crc = protocol_calc_crc(res_buf, offset);
  res_buf[offset++] = (crc / 10) + '0';
  res_buf[offset++] = (crc % 10) + '0';
  res_buf[offset++] = 0x0D;
  proto_hw_send(res_buf, offset);
}

void protocol_enable_rx(void) { uart_irq_rx_enable(g_proto_uart); }

void protocol_disable_rx(void) { uart_irq_rx_disable(g_proto_uart); }

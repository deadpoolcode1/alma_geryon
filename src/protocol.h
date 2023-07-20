#ifndef PROTOCOL_H_
#define PROTOCOL_H_

enum {
    SYNC_CH_1, 
    SYNC_CH_2,
    SYNC_CH_3,
    SYNC_MAX_CH,
};

enum proto_req_cmd {
    PROTO_REQ_CMD_SET_FREQ,
    PROTO_REQ_CMD_GET_FREQ,
    PROTO_REQ_CMD_SET_DUTY,
    PROTO_REQ_CMD_GET_DUTY,
    PROTO_REQ_CMD_SET_CURRENT,
    PROTO_REQ_CMD_GET_CURRENT,
    PROTO_REQ_CMD_SET_OPERATION,
    PROTO_REQ_CMD_SET_DRV_PULSE_MODE,
    PROTO_REQ_CMD_SET_SYNC_FREQ,
    PROTO_REQ_CMD_SET_JITTER,
    PROTO_REQ_CMD_SET_FAN_OPERATION,
    PROTO_REQ_CMD_SET_KEEP_ALIVE,
    PROTO_REQ_CMD_GET_ERROR,
    PROTO_REQ_CMD_GET_VERSION,
    PROTO_REQ_CMD_MAX,
};

enum proto_res_cmd {
    PROTO_RES_CMD_CUR_FREQ,
    PROTO_RES_CMD_CUR_DUTY,
    PROTO_RES_CMD_CUR_CHANNEL_CURRENT,
    PROTO_RES_CMD_KEEP_ALIVE_MSG,
    PROTO_RES_CMD_ERROR_MSG,
    PROTO_RES_CMD_SW_VERSIOn
};

struct proto_data {
    uint8_t freq;
    uint8_t duty[SYNC_MAX_CH];
    uint8_t current[SYNC_MAX_CH];
    uint8_t operation;
    uint8_t drive_pulse_mode;
    uint8_t sync_freq;
    uint8_t jitter_value;
    uint8_t fan_operation;
    uint8_t error;
};

enum proto_error_message {
    COMM_ERR = 0,
    OT_ERR = 1,
    FLT_ERR = 2,
    OC1_ERR = 3,
    OC2_ERR = 4,
    OV1_ERR = 5,
    OV2_ERR = 6,
};

void protocol_init(void);
void protocol_enable_rx(void);
void protocol_disable_rx(void);
void protop_send_error_message(uint8_t error);

#endif /* PROTOCOL_H_ */
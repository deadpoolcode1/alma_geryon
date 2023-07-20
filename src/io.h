#ifndef IO_H_
#define IO_H_

#include <stdint.h>

enum {
    IO_GLOBAL_EN,
    IO_STATUS_OUT,
    IO_RESET_INT,
    IO_FAN_OUT,
    IO_OT_INT,
    IO_OC2_INT,
    IO_OV2_INT,
    IO_PULSE2_OUT,
    IO_PWM2_OUT,
    IO_STATE2_OUT,
    IO_STATE3_OUT,
    IO_DRV_EN2_OUT,
    IO_DRV_EN1_OUT,
    IO_RS232_SHDN_OUT,
    IO_RS232_EN_OUT,
    IO_RESET_N_OUT,
    IO_STATE1_OUT,
    IO_FLT_INT,
    IO_PULSE1_OUT,
    IO_PWM1_OUT,
    IO_OC1_INT,
    IO_OV1_INT,
};

int io_init(void);
void io_set(int id, int level);
int io_get(int id);

#endif /* IO_H_ */
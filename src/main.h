#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>

struct alma_data {
    uint32_t sync_freq;
    uint8_t jitter;
    uint32_t pulse_freq;
    uint8_t pulse_dc[2];
    uint8_t op_mode;
    uint8_t pulse_mode;
};

#endif /* MAIN_H_ */
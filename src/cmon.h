#ifndef CMON_H_
#define CMON_H_

#include <stdint.h>

enum {
    CMON_CH_1 = 0, 
    CMON_CH_2 = 1,
    E_SENSE = 2,
};

void cmon_init(void);
int cmon_get(int channel);

#endif /* CMON_H_ */
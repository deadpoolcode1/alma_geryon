#ifndef CSET_H_
#define CSET_H_

#include <stdint.h>

enum {
    CSET_CH_1,
    CSET_CH_2
};

void cset_init(void);
int  cset_get_last_value(int channel);
void cset_out(int channel, int value);

#endif /* CSET_H_ */
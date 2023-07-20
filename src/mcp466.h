#ifndef MCP466_H_
#define MCP466_H_

#include <stdint.h>

enum {
    MCP466_CH_1 = 0b0101100 << 1,
    MCP466_CH_2 = 0b0101101 << 1,
};

void mcp466_init(void);
void mcp466_write(int channel, uint8_t reg, uint16_t value);
uint16_t mcp466_read(int channel, uint8_t reg);
void mcp466_up(int channel, uint8_t reg);
void mcp466_down(int channel, uint8_t reg);

#endif /* MCP466_H_ */
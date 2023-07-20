#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(mcp466_alma, LOG_LEVEL_DBG);

static const struct device *const mcp466_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));

void mcp466_init(void) {
    if (!device_is_ready(mcp466_dev)) {
		LOG_ERR("MCP466 device %s is not ready", mcp466_dev->name);
		return;
    }
}

void mcp466_write(int channel, uint8_t reg, uint16_t value) {
    if (!device_is_ready(mcp466_dev)) {
		LOG_ERR("MCP466 device %s is not ready", mcp466_dev->name);
		return;
    }

    uint8_t first_cmd_byte = 0x00;
    uint8_t second_cmd_byte = 0x00;
    uint16_t temp_data = value;
    uint8_t temp_byte = 0;
    // Prep the command bytes to write to the digital potentiometer
    reg *= 16;                // Shift the value of reg to the left by four bits
    first_cmd_byte |= reg;  // Load the register address into the first_cmd_byte
    temp_data &= 0x0100;            // Clear the top 7 bits and the lower byte of the input value to pick up the two data bits
    temp_data /= 256;               // Shift the top byte of the input value to the right by one byte
    temp_byte = (uint8_t)(temp_data);     // Store the top byte of the input value in a byte sized variable
    first_cmd_byte |= temp_byte;  // Load the two top input data bits into the first_cmd_byte
    temp_data = value;              // Load the input value into the temp_data
    temp_data &= 0x00FF;              // Clear the top byte
    second_cmd_byte = (uint8_t)(temp_data);  // Store the lower byte of the input value in the second_cmd_byte

    uint8_t tx_buf[2] = {first_cmd_byte, second_cmd_byte};
    i2c_write(mcp466_dev, tx_buf, sizeof(tx_buf), channel);
}

uint16_t mcp466_read(int channel, uint8_t reg) {
    if (!device_is_ready(mcp466_dev)) {
		LOG_ERR("MCP466 device %s is not ready", mcp466_dev->name);
		return 0;
    }

    uint8_t first_cmd_byte = 0x0C; // Prep the first_cmd_byte with 11b for reads
    // Prep the command bytes to read from the digital potentiometer
    reg *= 16; // Shift the value of Register to the left by four bits
    first_cmd_byte |= reg; // Load the register address into the first_cmd_byte

    uint8_t tx_buf[1] = {first_cmd_byte};
    uint8_t rx_buf[2] = {0x00};
    i2c_write_read(mcp466_dev, channel, tx_buf, sizeof(tx_buf), rx_buf, sizeof(rx_buf));

    return (uint16_t)(rx_buf[0] << 8 | rx_buf[1]);
}

void mcp466_up(int channel, uint8_t reg) {
    if (!device_is_ready(mcp466_dev)) {
		LOG_ERR("MCP466 device %s is not ready", mcp466_dev->name);
		return;
    }

    uint8_t first_cmd_byte = 0x04; // Prep the first_cmd_byte 01b to increment the reg 
    // Prep the command bytes to read from the digital potentiometer
    reg *= 16; // Shift the value of Register to the left by four bits
    first_cmd_byte |= reg; // Load the register address into the first_cmd_byte

    uint8_t tx_buf[1] = {first_cmd_byte};
    i2c_write(mcp466_dev, tx_buf, sizeof(tx_buf), channel);
}

void mcp466_down(int channel, uint8_t reg) {
    if (!device_is_ready(mcp466_dev)) {
		LOG_ERR("MCP466 device %s is not ready", mcp466_dev->name);
		return;
    }

    uint8_t first_cmd_byte = 0x08; // Prep the first_cmd_byte 0x08 to decrement  the reg 
    // Prep the command bytes to read from the digital potentiometer
    reg *= 16; // Shift the value of Register to the left by four bits
    first_cmd_byte |= reg; // Load the register address into the first_cmd_byte

    uint8_t tx_buf[1] = {first_cmd_byte};
    i2c_write(mcp466_dev, tx_buf, sizeof(tx_buf), channel);
}


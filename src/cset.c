#include <zephyr/kernel.h>
#include <zephyr/drivers/dac.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(dac_alma, LOG_LEVEL_ERR);

static const struct device *const dac_dev = DEVICE_DT_GET(DT_NODELABEL(dac1));

static int last_ch_value[2] = {0, 0};

static const struct dac_channel_cfg dac_ch_cfg[2] = {
    {
        .channel_id = 1,
        .resolution = 12,
    },
    {
        .channel_id = 2,
        .resolution = 12
    }
};

void cset_init(void) {
	if (!device_is_ready(dac_dev)) {
		LOG_ERR("DAC device %s is not ready", dac_dev->name);
		return;
	}

	int ret = dac_channel_setup(dac_dev, dac_ch_cfg);

	if (ret != 0) {
		LOG_ERR("Setting up of DAC channel failed with code %d", ret);
		return;
	}

    LOG_INF("Setup DAC for CSET successfully");
}

void cset_out(int channel, int value) {
	if (!device_is_ready(dac_dev)) {
		LOG_ERR("DAC device %s is not ready", dac_dev->name);
		return;
	}

    last_ch_value[channel] = value;
    dac_write_value(dac_dev, dac_ch_cfg[channel].channel_id, value);
}

int  cset_get_last_value(int channel) {
    return last_ch_value[channel];
}
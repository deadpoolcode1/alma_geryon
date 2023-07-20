#include <zephyr/kernel.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(adc_alma, LOG_LEVEL_DBG);

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
	!DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
	ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

/* Data of ADC io-channels specified in devicetree. */
static const struct adc_dt_spec adc_channels[] = {
	DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
			     DT_SPEC_AND_COMMA)
};

static int16_t adc_buf;
struct adc_sequence sequence = {
	.buffer = &adc_buf,
	/* buffer size in bytes, not number of samples */
	.buffer_size = sizeof(adc_buf),
};


void cmon_init(void) {

	/* Configure channels individually prior to sampling. */
	for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
		if (!device_is_ready(adc_channels[i].dev)) {
			LOG_ERR("ADC controller device not ready");
			return;
		}

		int err = adc_channel_setup_dt(&adc_channels[i]);
		if (err < 0) {
			LOG_ERR("Could not setup channel #%d (%d)", i, err);
			return;
		}
	}

    LOG_INF("Setup ADC for CMON successfully");
}

int cmon_get(int channel) {
	int32_t val_mv;
	(void)adc_sequence_init_dt(&adc_channels[channel], &sequence);

	int err = adc_read(adc_channels[channel].dev, &sequence);
	if (err < 0) {
		LOG_ERR("Could not read (%d)", err);
        return 0;
	} else {
		LOG_DBG("Raw buf: %d", adc_buf);
	}

	/* conversion to mV may not be supported, skip if not */
	val_mv = adc_buf;
	err = adc_raw_to_millivolts_dt(&adc_channels[channel],
				       &val_mv);
	if (err < 0) {
		LOG_DBG(" (value in mV not available)");
	} else {
		LOG_INF(" Voltage: %d mV", val_mv);
	}

    return val_mv;
}
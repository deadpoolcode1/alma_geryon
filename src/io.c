#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/sys/slist.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(io, LOG_LEVEL_ERR);

#include "io.h"
#include "app_event_manager.h"

static const struct gpio_dt_spec global_en_dt = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(global_en), control_gpios, 0);
static const struct gpio_dt_spec status_out_dt = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(status_out), control_gpios, 0);
static const struct gpio_dt_spec reset_int_dt = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(reset_int), control_gpios, 0);
static const struct gpio_dt_spec fan_out_dt = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(fan_out), control_gpios, 0);
static const struct gpio_dt_spec ot_int_dt = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(ot_int), control_gpios, 0);
static const struct gpio_dt_spec oc2_int_dt = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(oc2_int), control_gpios, 0);
static const struct gpio_dt_spec ov2_int_dt = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(ov2_int), control_gpios, 0);
static const struct gpio_dt_spec pulse2_out_dt = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(pulse2_out), control_gpios, 0);
static const struct gpio_dt_spec pwm2_out_dt = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(pwm2_out), control_gpios, 0);
static const struct gpio_dt_spec state2_out_dt = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(state2_out), control_gpios, 0);
static const struct gpio_dt_spec state3_out_dt = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(state3_out), control_gpios, 0);
static const struct gpio_dt_spec drv_en2_out_dt = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(drv_en2_out), control_gpios, 0);
static const struct gpio_dt_spec drv_en1_out_dt = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(drv_en1_out), control_gpios, 0);
static const struct gpio_dt_spec rs232_shdn_out_dt = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(rs232_shdn_out), control_gpios, 0);
static const struct gpio_dt_spec rs232_en_out_dt = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(rs232_en_out), control_gpios, 0);
static const struct gpio_dt_spec reset_n_out_dt = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(reset_n_out), control_gpios, 0);
static const struct gpio_dt_spec state1_out_dt = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(state1_out), control_gpios, 0);
static const struct gpio_dt_spec flt_int_dt = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(flt_int), control_gpios, 0);
static const struct gpio_dt_spec pulse1_out_dt = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(pulse1_out), control_gpios, 0);
static const struct gpio_dt_spec pwm1_out_dt = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(pwm1_out), control_gpios, 0);
static const struct gpio_dt_spec oc1_int_dt = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(oc1_int), control_gpios, 0);
static const struct gpio_dt_spec ov1_int_dt = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(ov1_int), control_gpios, 0);

static struct gpio_callback global_en_cb;
static struct gpio_callback reset_int_cb;
static struct gpio_callback ot_int_cb;
static struct gpio_callback oc2_int_cb;
static struct gpio_callback ov2_int_cb;
static struct gpio_callback flt_int_cb;
static struct gpio_callback oc1_int_cb;
static struct gpio_callback ov1_int_cb;

static void global_en_callback_handler(const struct device *port, struct gpio_callback *cb, gpio_port_pins_t pins) {
	struct app_event event;
	uint8_t level = gpio_pin_get_dt(&global_en_dt);
	LOG_DBG("Level %d", level);
	if (level == 0) {
		event.type = APP_EVENT_IN_GLOBAL_EN_FALLING;
	} else if (level == 1) {
		event.type = APP_EVENT_IN_GLOBAL_EN_RISING;
	} else {
		event.type = APP_EVENT_IN_GLOBAL_EN_FALLING;
	}
	app_event_manager_push(&event);
}

static void reset_int_callback_handler(const struct device *port, struct gpio_callback *cb, gpio_port_pins_t pins) {
	struct app_event event = {
		.type = APP_EVENT_IN_RESET,
	};
    app_event_manager_push(&event);
}

static void ot_int_callback_handler(const struct device *port, struct gpio_callback *cb, gpio_port_pins_t pins) {
	struct app_event event = {
		.type = APP_EVENT_IN_OT,
	};
    app_event_manager_push(&event);
}

static void oc2_int_callback_handler(const struct device *port, struct gpio_callback *cb, gpio_port_pins_t pins) {
	struct app_event event = {
		.type = APP_EVENT_IN_OC2,
	};
    app_event_manager_push(&event);
}

static void ov2_int_callback_handler(const struct device *port, struct gpio_callback *cb, gpio_port_pins_t pins) {
	struct app_event event = {
		.type = APP_EVENT_IN_OV2,
	};
    app_event_manager_push(&event);
}

static void flt_int_callback_handler(const struct device *port, struct gpio_callback *cb, gpio_port_pins_t pins) {
	struct app_event event = {
		.type = APP_EVENT_IN_FLT,
	};
    app_event_manager_push(&event);
}

static void oc1_int_callback_handler(const struct device *port, struct gpio_callback *cb, gpio_port_pins_t pins) {
	struct app_event event = {
		.type = APP_EVENT_IN_OV1,
	};
    app_event_manager_push(&event);
}

static void ov1_int_callback_handler(const struct device *port, struct gpio_callback *cb, gpio_port_pins_t pins) {
	struct app_event event = {
		.type = APP_EVENT_IN_OC1,
	};
    app_event_manager_push(&event);
}

int io_init(void)
{
	if (!device_is_ready(global_en_dt.port)) {
		LOG_ERR("Can't get the device %s", global_en_dt.port->name);
		return -EINVAL;
	}

	if (!device_is_ready(status_out_dt.port)) {
		LOG_ERR("Can't get the device %s", status_out_dt.port->name);
		return -EINVAL;
	}

	if (!device_is_ready(reset_int_dt.port)) {
		LOG_ERR("Can't get the device %s", reset_int_dt.port->name);
		return -EINVAL;
	}
    
	if (!device_is_ready(fan_out_dt.port)) {
		LOG_ERR("Can't get the device %s", fan_out_dt.port->name);
		return -EINVAL;
	}

	if (!device_is_ready(ot_int_dt.port)) {
		LOG_ERR("Can't get the device %s", ot_int_dt.port->name);
		return -EINVAL;
	}

	if (!device_is_ready(oc2_int_dt.port)) {
		LOG_ERR("Can't get the device %s", oc2_int_dt.port->name);
		return -EINVAL;
	}

	if (!device_is_ready(ov2_int_dt.port)) {
		LOG_ERR("Can't get the device %s", ov2_int_dt.port->name);
		return -EINVAL;
	}

	if (!device_is_ready(pulse2_out_dt.port)) {
		LOG_ERR("Can't get the device %s", pulse2_out_dt.port->name);
		return -EINVAL;
	}

	if (!device_is_ready(pwm2_out_dt.port)) {
		LOG_ERR("Can't get the device %s", pwm2_out_dt.port->name);
		return -EINVAL;
	}

	if (!device_is_ready(state2_out_dt.port)) {
		LOG_ERR("Can't get the device %s", state2_out_dt.port->name);
		return -EINVAL;
	}

	if (!device_is_ready(state3_out_dt.port)) {
		LOG_ERR("Can't get the device %s", state3_out_dt.port->name);
		return -EINVAL;
	}

	if (!device_is_ready(drv_en2_out_dt.port)) {
		LOG_ERR("Can't get the device %s", drv_en2_out_dt.port->name);
		return -EINVAL;
	}

	if (!device_is_ready(drv_en1_out_dt.port)) {
		LOG_ERR("Can't get the device %s", drv_en1_out_dt.port->name);
		return -EINVAL;
	}

	if (!device_is_ready(rs232_shdn_out_dt.port)) {
		LOG_ERR("Can't get the device %s", rs232_shdn_out_dt.port->name);
		return -EINVAL;
	}

	if (!device_is_ready(rs232_en_out_dt.port)) {
		LOG_ERR("Can't get the device %s", rs232_en_out_dt.port->name);
		return -EINVAL;
	}

	if (!device_is_ready(reset_n_out_dt.port)) {
		LOG_ERR("Can't get the device %s", reset_n_out_dt.port->name);
		return -EINVAL;
	}

	if (!device_is_ready(state1_out_dt.port)) {
		LOG_ERR("Can't get the device %s", state1_out_dt.port->name);
		return -EINVAL;
	}

	if (!device_is_ready(flt_int_dt.port)) {
		LOG_ERR("Can't get the device %s", flt_int_dt.port->name);
		return -EINVAL;
	}

	if (!device_is_ready(pulse1_out_dt.port)) {
		LOG_ERR("Can't get the device %s", pulse1_out_dt.port->name);
		return -EINVAL;
	}

	if (!device_is_ready(pwm1_out_dt.port)) {
		LOG_ERR("Can't get the device %s", pwm1_out_dt.port->name);
		return -EINVAL;
	}

	if (!device_is_ready(oc1_int_dt.port)) {
		LOG_ERR("Can't get the device %s", oc1_int_dt.port->name);
		return -EINVAL;
	}

	if (!device_is_ready(ov1_int_dt.port)) {
		LOG_ERR("Can't get the device %s", ov1_int_dt.port->name);
		return -EINVAL;
	}

    gpio_pin_configure_dt(&global_en_dt, GPIO_INPUT | GPIO_PULL_UP);
    gpio_pin_configure_dt(&status_out_dt, GPIO_OUTPUT);
    gpio_pin_configure_dt(&reset_int_dt, GPIO_INPUT);
    gpio_pin_configure_dt(&fan_out_dt, GPIO_OUTPUT);
    gpio_pin_configure_dt(&ot_int_dt, GPIO_INPUT);
    gpio_pin_configure_dt(&oc2_int_dt, GPIO_INPUT);
    gpio_pin_configure_dt(&ov2_int_dt, GPIO_INPUT);
    gpio_pin_configure_dt(&pulse2_out_dt, GPIO_OUTPUT);
    gpio_pin_configure_dt(&pwm2_out_dt, GPIO_OUTPUT);
    gpio_pin_configure_dt(&state2_out_dt, GPIO_OUTPUT);
    gpio_pin_configure_dt(&state3_out_dt, GPIO_OUTPUT);
    gpio_pin_configure_dt(&drv_en2_out_dt, GPIO_OUTPUT);
    gpio_pin_configure_dt(&drv_en1_out_dt, GPIO_OUTPUT);
    gpio_pin_configure_dt(&rs232_shdn_out_dt, GPIO_OUTPUT);
    gpio_pin_configure_dt(&rs232_en_out_dt, GPIO_OUTPUT);
    gpio_pin_configure_dt(&reset_n_out_dt, GPIO_OUTPUT);
    gpio_pin_configure_dt(&state1_out_dt, GPIO_OUTPUT);
    gpio_pin_configure_dt(&flt_int_dt, GPIO_INPUT);
    gpio_pin_configure_dt(&pulse1_out_dt, GPIO_OUTPUT);
    gpio_pin_configure_dt(&pwm1_out_dt, GPIO_OUTPUT);
    gpio_pin_configure_dt(&oc1_int_dt, GPIO_INPUT);
    gpio_pin_configure_dt(&ov1_int_dt, GPIO_INPUT);

    /* RS232 SHDN -> constant high by default */
    gpio_pin_set_dt(&rs232_shdn_out_dt, 1U);
    /* RS232 EN -> constant low by default */
    gpio_pin_set_dt(&rs232_en_out_dt, 0U);

    gpio_pin_interrupt_configure_dt(&global_en_dt, GPIO_INT_EDGE_BOTH);
	gpio_init_callback(&global_en_cb, global_en_callback_handler, BIT(global_en_dt.pin));
	gpio_add_callback(global_en_dt.port, &global_en_cb);

    gpio_pin_interrupt_configure_dt(&reset_int_dt, GPIO_INT_EDGE_RISING);
	gpio_init_callback(&reset_int_cb, reset_int_callback_handler, BIT(reset_int_dt.pin));
	gpio_add_callback(reset_int_dt.port, &reset_int_cb);

    gpio_pin_interrupt_configure_dt(&ot_int_dt, GPIO_INT_EDGE_RISING);
	gpio_init_callback(&ot_int_cb, ot_int_callback_handler, BIT(ot_int_dt.pin));
	gpio_add_callback(ot_int_dt.port, &ot_int_cb);

    gpio_pin_interrupt_configure_dt(&oc2_int_dt, GPIO_INT_EDGE_RISING);
	gpio_init_callback(&oc2_int_cb, oc2_int_callback_handler, BIT(oc2_int_dt.pin));
	gpio_add_callback(oc2_int_dt.port, &oc2_int_cb);

    gpio_pin_interrupt_configure_dt(&ov2_int_dt, GPIO_INT_EDGE_RISING);
	gpio_init_callback(&ov2_int_cb, ov2_int_callback_handler, BIT(ov2_int_dt.pin));
	gpio_add_callback(ov2_int_dt.port, &ov2_int_cb);

    gpio_pin_interrupt_configure_dt(&flt_int_dt, GPIO_INT_EDGE_RISING);
	gpio_init_callback(&flt_int_cb, flt_int_callback_handler, BIT(flt_int_dt.pin));
	gpio_add_callback(flt_int_dt.port, &flt_int_cb);

    gpio_pin_interrupt_configure_dt(&oc1_int_dt, GPIO_INT_EDGE_RISING);
	gpio_init_callback(&oc1_int_cb, oc1_int_callback_handler, BIT(oc1_int_dt.pin));
	gpio_add_callback(oc1_int_dt.port, &oc1_int_cb);

    gpio_pin_interrupt_configure_dt(&ov1_int_dt, GPIO_INT_EDGE_RISING);
	gpio_init_callback(&ov1_int_cb, ov1_int_callback_handler, BIT(ov1_int_dt.pin));
	gpio_add_callback(ov1_int_dt.port, &ov1_int_cb);

	LOG_DBG("Initialized the IO Interface successfully");
	return 0;
}

void io_set(int id, int level) {
	switch (id) {
	case IO_STATUS_OUT:
		gpio_pin_set_dt(&status_out_dt, level);
		break;
	case IO_FAN_OUT:
		gpio_pin_set_dt(&fan_out_dt, level);
		break;
	case IO_PULSE2_OUT:
		gpio_pin_set_dt(&pulse2_out_dt, level);
		break;
	case IO_PWM2_OUT:
		gpio_pin_set_dt(&pwm2_out_dt, level);
		break;
	case IO_STATE2_OUT:
		gpio_pin_set_dt(&state2_out_dt, level);
		break;
	case IO_STATE3_OUT:
		gpio_pin_set_dt(&state3_out_dt, level);
		break;
	case IO_DRV_EN2_OUT:
		gpio_pin_set_dt(&drv_en2_out_dt, level);
		break;
	case IO_DRV_EN1_OUT:
		gpio_pin_set_dt(&drv_en1_out_dt, level);
		break;
	case IO_RS232_SHDN_OUT:
		gpio_pin_set_dt(&rs232_shdn_out_dt, level);
		break;
	case IO_RESET_N_OUT:
		gpio_pin_set_dt(&reset_n_out_dt, level);
		break;
	case IO_RS232_EN_OUT:
		gpio_pin_set_dt(&rs232_en_out_dt, level);
		break;
	case IO_STATE1_OUT:
		gpio_pin_set_dt(&state1_out_dt, level);
		break;
	case IO_PULSE1_OUT:
		gpio_pin_set_dt(&pulse1_out_dt, level);
		break;
	case IO_PWM1_OUT:
		gpio_pin_set_dt(&pwm1_out_dt, level);
		break;
	default:
		break;
	}
}

int io_get(int id) {
	switch (id) {
		case IO_GLOBAL_EN:
			return gpio_pin_get_dt(&global_en_dt);
		case IO_STATUS_OUT:
			return gpio_pin_get_dt(&status_out_dt);
		case IO_RESET_INT:
			return gpio_pin_get_dt(&reset_int_dt);
		case IO_FAN_OUT:
			return gpio_pin_get_dt(&fan_out_dt);
		case IO_OT_INT:
			return gpio_pin_get_dt(&ot_int_dt);
		case IO_OC2_INT:
			return gpio_pin_get_dt(&oc2_int_dt);
		case IO_OV2_INT:
			return gpio_pin_get_dt(&ov2_int_dt);
		case IO_PULSE2_OUT:
			return gpio_pin_get_dt(&pulse2_out_dt);
		case IO_PWM2_OUT:
			return gpio_pin_get_dt(&pwm2_out_dt);
		case IO_STATE2_OUT:
			return gpio_pin_get_dt(&state2_out_dt);
		case IO_STATE3_OUT:
			return gpio_pin_get_dt(&state3_out_dt);
		case IO_DRV_EN2_OUT:
			return gpio_pin_get_dt(&drv_en2_out_dt);
		case IO_DRV_EN1_OUT:
			return gpio_pin_get_dt(&drv_en1_out_dt);
		case IO_RS232_SHDN_OUT:
			return gpio_pin_get_dt(&rs232_shdn_out_dt);
		case IO_RS232_EN_OUT:
			return gpio_pin_get_dt(&rs232_en_out_dt);
		case IO_RESET_N_OUT:
			return gpio_pin_get_dt(&reset_n_out_dt);
		case IO_STATE1_OUT:
			return gpio_pin_get_dt(&state1_out_dt);
		case IO_FLT_INT:
			return gpio_pin_get_dt(&flt_int_dt);
		case IO_PULSE1_OUT:
			return gpio_pin_get_dt(&pulse1_out_dt);
		case IO_PWM1_OUT:
			return gpio_pin_get_dt(&pwm1_out_dt);
		case IO_OC1_INT:
			return gpio_pin_get_dt(&oc1_int_dt);
		case IO_OV1_INT:
			return gpio_pin_get_dt(&ov1_int_dt);
	}
	return -EINVAL;
}
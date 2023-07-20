#include <zephyr/kernel.h>
#include "app_event_manager.h"
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_event_manager);

/* Define message queue */
K_MSGQ_DEFINE(app_event_msq, sizeof(struct app_event), APP_EVENT_QUEUE_SIZE, 4);

int app_event_manager_push(struct app_event *p_evt)
{
	return k_msgq_put(&app_event_msq, p_evt, K_NO_WAIT);
}

int app_event_manager_get(const struct app_event *p_evt)
{
	return k_msgq_get(&app_event_msq, (struct app_event *)p_evt, K_FOREVER);
}

char *app_event_type_to_string(enum app_event_type type)
{
	switch (type) {
		case APP_EVENT_IN_GLOBAL_EN_RISING:
			return "APP_EVENT_IN_GLOBAL_EN_RISING";
		case APP_EVENT_IN_GLOBAL_EN_FALLING:
			return "APP_EVENT_IN_GLOBAL_EN_FALLING";
		case APP_EVENT_IN_RESET:
			return "APP_EVENT_IN_RESET";
		case APP_EVENT_IN_OT:
			return "APP_EVENT_IN_OT";
		case APP_EVENT_IN_OC2:
			return "APP_EVENT_IN_OC2";
		case APP_EVENT_IN_OV2:
			return "APP_EVENT_IN_OV2";
		case APP_EVENT_IN_FLT:
			return "APP_EVENT_IN_FLT";
		case APP_EVENT_IN_OV1:
			return "APP_EVENT_IN_OV1";
		case APP_EVENT_IN_OC1:
			return "APP_EVENT_IN_OC1";
		default:
			return "UNKNOWN";
	}
}
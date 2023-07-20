#ifndef _APP_EVENT_MANAGER_H
#define _APP_EVENT_MANAGER_H

/**
 * @brief Max size of event queue
 * 
 */
#define APP_EVENT_QUEUE_SIZE 24

/**
 * @brief Simplified macro for pushing an app event without data
 * 
 */
#define APP_EVENT_MANAGER_PUSH(x)                                                                  \
	struct app_event app_event_##x = {                                                         \
		.type = x,                                                                         \
	};                                                                                         \
	app_event_manager_push(&app_event_##x);

/**
 * @brief Used to interact with different functionality
 * in this application
 */
enum app_event_type {
	APP_EVENT_IN_GLOBAL_EN_RISING,
	APP_EVENT_IN_GLOBAL_EN_FALLING,
	APP_EVENT_IN_RESET,
	APP_EVENT_IN_OT,
	APP_EVENT_IN_OC2,
	APP_EVENT_IN_OV2,
	APP_EVENT_IN_FLT,
	APP_EVENT_IN_OV1,
	APP_EVENT_IN_OC1,
};

/**
 * @brief Application event that can be passed back to the main
 * context
 * 
 */
struct app_event {
	enum app_event_type type;
	union {
		int err;
	};
};

/**
 * @brief Get the string representation of the Application event
 * 
 * @param type app event type enum
 * @return char* NULL if error
 */
char *app_event_type_to_string(enum app_event_type type);

/**
 * @brief Pushes event to message queue
 * 
 * @param p_evt the event to be copied.
 * @return int 
 */
int app_event_manager_push(struct app_event *p_evt);

/**
 * @brief Gets an event from the message queue
 * 
 * @param p_evt pointer where the data will be copied to.
 * @return int 
 */
int app_event_manager_get(const struct app_event *p_evt);

#endif
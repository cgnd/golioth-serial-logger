/*
 * Copyright (c) 2024 Common Ground Electronics
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <golioth/client.h>
#include <samples/common/net_connect.h>
#include <samples/common/sample_credentials.h>

#include "serial_listener.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

struct golioth_client *client;

static void on_client_event(struct golioth_client *client, enum golioth_client_event event,
			    void *arg)
{
	switch (event) {
	case GOLIOTH_CLIENT_EVENT_CONNECTED:
		LOG_INF("Golioth client connected");
		break;
	case GOLIOTH_CLIENT_EVENT_DISCONNECTED:
		LOG_INF("Golioth client disconnected");
		break;
	}
}

int main(void)
{
	int ret;
	char msg[LOG_MSG_SIZE];

	LOG_INF("Starting Golioth Serial Logger App");

	/* Initialize the serial listener */
	if (serial_listener_init()) {
		return 0;
	}

	/* Connect to WiFi */
	net_connect();

	/* Initialize the Golioth client */
	const struct golioth_client_config *client_config = golioth_sample_credentials_get();
	client = golioth_client_create(client_config);
	golioth_client_register_event_callback(client, on_client_event, NULL);

	/* Process any log messages in the queue */
	while (k_msgq_get(&uart_msgq, &msg, K_FOREVER) == 0) {
		/* Wait forever for the Golioth client to be connected */
		golioth_client_wait_for_connect(client, -1);

		/* The Golioth logging backend will send this message to the Golioth cloud */
		LOG_INF("target_device: %s", msg);
	}

	return 0;
}

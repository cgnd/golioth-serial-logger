/*
 * Copyright (c) 2022 Libre Solar Technologies GmbH
 * Copyright (c) 2024 Common Ground Electronics
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>

#include "serial_listener.h"

#define UART_DEVICE_NODE DT_ALIAS(target_uart)
static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

LOG_MODULE_REGISTER(serial_listener, LOG_LEVEL_DBG);

/* queue to store log messages */
#define LOG_MSG_QUEUE_SIZE 100
K_MSGQ_DEFINE(uart_msgq, LOG_MSG_SIZE, LOG_MSG_QUEUE_SIZE, 1);

/* receive buffer used in UART ISR callback */
static char rx_buf[LOG_MSG_SIZE];
static int rx_buf_pos;

/*
 * Read characters from UART until line end is detected. Afterwards push the
 * data to the message queue.
 */
void serial_cb(const struct device *dev, void *user_data)
{
	uint8_t c;

	if (!uart_irq_update(uart_dev)) {
		return;
	}

	if (!uart_irq_rx_ready(uart_dev)) {
		return;
	}

	/* read until FIFO empty */
	while (uart_fifo_read(uart_dev, &c, 1) == 1) {
		if ((c == '\n' || c == '\r') && rx_buf_pos > 0) {
			/* terminate string */
			rx_buf[rx_buf_pos] = '\0';

			/* if queue is full, message is silently dropped */
			k_msgq_put(&uart_msgq, &rx_buf, K_NO_WAIT);

			/* reset the buffer (it was copied to the msgq) */
			rx_buf_pos = 0;
		} else if (rx_buf_pos < (sizeof(rx_buf) - 1)) {
			rx_buf[rx_buf_pos++] = c;
		}
		/* else: characters beyond buffer size are dropped */
	}
}

int serial_listener_init(void)
{
	int ret;

	/* Make sure the UART device exists and is ready */
	if (!device_is_ready(uart_dev)) {
		printk("UART device is not ready!");
		return -ENODEV;
	}

	/* Configure UART interrupt and callback to receive data */
	ret = uart_irq_callback_user_data_set(uart_dev, serial_cb, NULL);
	if (ret < 0) {
		if (ret == -ENOTSUP) {
			printk("Interrupt-driven UART API support not enabled\n");
		} else if (ret == -ENOSYS) {
			printk("UART device does not support interrupt-driven API\n");
		} else {
			printk("Error setting UART callback: %d\n", ret);
		}
		return ret;
	}
	uart_irq_rx_enable(uart_dev);

	return 0;
}

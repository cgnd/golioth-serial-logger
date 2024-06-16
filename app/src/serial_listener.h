/*
 * Copyright (c) 2024 Common Ground Electronics
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_MSG_SIZE 256

extern struct k_msgq uart_msgq;

int serial_listener_init(void);

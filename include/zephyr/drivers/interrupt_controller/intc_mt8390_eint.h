/*
 * Copyright 2025 MediaTek
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_DRIVERS_INTERRUPT_CONTROLLER_INTC_mt8390_EINT_H_
#define ZEPHYR_DRIVERS_INTERRUPT_CONTROLLER_INTC_mt8390_EINT_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*eint_mtk_cb_handler_t)(const struct device *dev, uint8_t line, void *arg);

typedef struct {
	sys_snode_t node;
	eint_mtk_cb_handler_t cb_handler;
	const struct device *cb_dev;
	void *cb_arg;
	uint8_t first_line;
	uint8_t num_lines;
} eint_mtk_callback_t;

int eint_mtk_init_callback(eint_mtk_callback_t *callback, uint8_t first_line, uint8_t num_lines,
			   eint_mtk_cb_handler_t cb_handler, const struct device *cb_dev,
			   void *cb_arg);
int eint_mtk_add_callback(const struct device *dev, eint_mtk_callback_t *callback);
void eint_mtk_remove_callback(const struct device *dev, eint_mtk_callback_t *callback);

int eint_mtk_enable(const struct device *dev, uint8_t line);
int eint_mtk_disable(const struct device *dev, uint8_t line);
bool eint_mtk_is_enabled(const struct device *dev, uint8_t line);

#ifdef __cplusplus
}
#endif
#endif /* ZEPHYR_DRIVERS_INTERRUPT_CONTROLLER_INTC_mt8390_EINT_H_ */

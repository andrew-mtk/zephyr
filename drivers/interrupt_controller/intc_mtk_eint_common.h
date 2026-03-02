/*
 * Copyright (c) 2026 MediaTek
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef DRIVERS_INTERRUPT_CONTROLLER_INTC_MTK_EINT_COMMON_H_
#define DRIVERS_INTERRUPT_CONTROLLER_INTC_MTK_EINT_COMMON_H_

#include <zephyr/device.h>
#include <zephyr/drivers/interrupt_controller/intc_mtk_eint.h>
#include <zephyr/spinlock.h>

typedef void (*irq_config_fct)(const struct device *dev);

typedef uint32_t (*line_to_bit_mask_fct)(uint8_t line);
typedef uint32_t (*offset_sta_fct)(uint8_t line);
typedef uint32_t (*offset_ack_fct)(uint8_t line);
typedef uint32_t (*offset_mask_fct)(uint8_t line);
typedef uint32_t (*offset_mask_set_fct)(uint8_t line);
typedef uint32_t (*offset_mask_clr_fct)(uint8_t line);

typedef struct {
	DEVICE_MMIO_ROM; /* Must be first */
	uint16_t num_lines;

	irq_config_fct irq_config;

	line_to_bit_mask_fct line_to_bit_mask;

	offset_sta_fct offset_sta;
	offset_ack_fct offset_ack;
	offset_mask_fct offset_mask;
	offset_mask_set_fct offset_mask_set;
	offset_mask_clr_fct offset_mask_clr;
} eint_mtk_config_t;

typedef struct {
	eint_mtk_callback_t callback;
	void *arg;
	uint8_t line;
} eint_callback_t;

typedef struct {
	DEVICE_MMIO_RAM; /* Must be first */

	struct k_spinlock lock;

	sys_slist_t eint_callbacks;
} eint_mtk_data_t;

extern void eint_mtk_isr(const struct device *dev);

extern int eint_mtk_init(const struct device *dev);

#endif /* DRIVERS_INTERRUPT_CONTROLLER_INTC_MTK_EINT_COMMON_H_ */

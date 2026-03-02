/*
 * Copyright (c) 2026 MediaTek
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "intc_mtk_eint_common.h"

static void handle_line_isr(const struct device *dev, uint8_t line, uint32_t bit_mask);

static void handle_line_isr(const struct device *dev, uint8_t line, uint32_t bit_mask)
{
	eint_mtk_data_t *eint_data = dev->data;
	eint_mtk_callback_t *eint_callback;
	eint_mtk_callback_t *tmp;
	const eint_mtk_config_t *eint_config = dev->config;

	while (bit_mask != 0) {
		if ((bit_mask & 0x00000001) != 0) {
			SYS_SLIST_FOR_EACH_CONTAINER_SAFE(&(eint_data->eint_callbacks),
							  eint_callback, tmp, node) {
				if ((line >= eint_callback->first_line) &&
				    (line <
				     (eint_callback->first_line + eint_callback->num_lines))) {
					eint_callback->cb_handler(eint_callback->cb_dev, line,
								  eint_callback->cb_arg);
				}
			}

			sys_write32(eint_config->line_to_bit_mask(line),
				    (DEVICE_MMIO_GET(dev) + eint_config->offset_ack(line)));
		}

		line++;
		bit_mask >>= 1;
	}
}

int eint_mtk_init_callback(eint_mtk_callback_t *callback, uint8_t first_line, uint8_t num_lines,
			   eint_mtk_cb_handler_t cb_handler, const struct device *cb_dev,
			   void *cb_arg)
{
	if ((callback == NULL) || (cb_handler == NULL) || (cb_dev == NULL)) {
		return -EINVAL;
	}

	callback->cb_handler = cb_handler;
	callback->cb_dev = cb_dev;
	callback->cb_arg = cb_arg;
	callback->first_line = first_line;
	callback->num_lines = num_lines;

	return 0;
}

int eint_mtk_add_callback(const struct device *dev, eint_mtk_callback_t *callback)
{
	sys_snode_t *prev_callback;
	eint_mtk_data_t *eint_data = dev->data;
	k_spinlock_key_t key;

	if ((callback == NULL) || (callback->cb_handler == NULL) || (callback->cb_dev == NULL)) {
		return -EINVAL;
	}

	key = k_spin_lock(&(eint_data->lock));

	if (sys_slist_find(&(eint_data->eint_callbacks), &(callback->node), &prev_callback)) {
		/* Duplicate callback setting. */
		k_spin_unlock(&(eint_data->lock), key);

		return -EINVAL;
	}

	sys_slist_prepend(&(eint_data->eint_callbacks), &(callback->node));

	k_spin_unlock(&(eint_data->lock), key);

	return 0;
}

void eint_mtk_remove_callback(const struct device *dev, eint_mtk_callback_t *callback)
{
	eint_mtk_data_t *eint_data = dev->data;
	k_spinlock_key_t key;

	key = k_spin_lock(&(eint_data->lock));

	sys_slist_find_and_remove(&(eint_data->eint_callbacks), &(callback->node));

	k_spin_unlock(&(eint_data->lock), key);
}

int eint_mtk_enable(const struct device *dev, uint8_t line)
{
	const eint_mtk_config_t *eint_config = dev->config;

	if (line >= eint_config->num_lines) {
		return -EINVAL;
	}

	sys_write32(eint_config->line_to_bit_mask(line),
		    (DEVICE_MMIO_GET(dev) + eint_config->offset_mask_set(line)));

	return 0;
}

int eint_mtk_disable(const struct device *dev, uint8_t line)
{
	const eint_mtk_config_t *eint_config = dev->config;

	if (line >= eint_config->num_lines) {
		return -EINVAL;
	}

	sys_write32(eint_config->line_to_bit_mask(line),
		    (DEVICE_MMIO_GET(dev) + eint_config->offset_mask_clr(line)));

	return 0;
}

bool eint_mtk_is_enabled(const struct device *dev, uint8_t line)
{
	uint32_t val;
	const eint_mtk_config_t *eint_config = dev->config;

	if (line >= eint_config->num_lines) {
		return (false);
	}

	val = sys_read32(DEVICE_MMIO_GET(dev) + eint_config->offset_mask(line));
	val &= eint_config->line_to_bit_mask(line);

	return (val != 0);
}

void eint_mtk_isr(const struct device *dev)
{
	uint8_t line = 0;
	uint32_t status;
	const eint_mtk_config_t *eint_config = dev->config;

	while (line < eint_config->num_lines) {
		status = sys_read32(DEVICE_MMIO_GET(dev) + eint_config->offset_sta(line));

		if (status != 0) {
			handle_line_isr(dev, line, status);
		}

		line += 32;
	}
}

int eint_mtk_init(const struct device *dev)
{
	eint_mtk_data_t *eint_data = dev->data;
	const eint_mtk_config_t *eint_config = dev->config;

	DEVICE_MMIO_MAP(dev, K_MEM_CACHE_NONE);

	sys_slist_init(&(eint_data->eint_callbacks));

	eint_config->irq_config(dev);

	return 0;
}

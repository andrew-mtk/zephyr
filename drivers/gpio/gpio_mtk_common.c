/*
 * Copyright (c) 2026 MediaTek
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/drivers/gpio/gpio_utils.h>

#include "gpio_mtk_common.h"

static int mmio_read32(const struct device *dev, reg_type_t reg_type, uint32_t *value);
static int mmio_write32(const struct device *dev, reg_type_t reg_type, uint32_t value);

static void eint_handler(const struct device *dev, uint8_t line, void *arg);

static int mmio_read32(const struct device *dev, reg_type_t reg_type, uint32_t *value)
{
	uint32_t offset;
	const gpio_mtk_config_t *gpio_config = dev->config;

	offset = gpio_config->reg_offset(dev, reg_type);
	if (offset == INV_REG_OFFSET) {
		return -EINVAL;
	}

	(*value) = sys_read32(DEVICE_MMIO_NAMED_GET(dev, reg_base) + offset) &
		   gpio_config->gpio_pin_mask;

	return 0;
}

static int mmio_write32(const struct device *dev, reg_type_t reg_type, uint32_t value)
{
	uint32_t offset;
	const gpio_mtk_config_t *gpio_config = dev->config;

	offset = gpio_config->reg_offset(dev, reg_type);
	if (offset == INV_REG_OFFSET) {
		return -EINVAL;
	}

	sys_write32(value, DEVICE_MMIO_NAMED_GET(dev, reg_base) + offset);

	return 0;
}

static void eint_handler(const struct device *dev, uint8_t line, void *arg)
{
	gpio_mtk_data_t *gpio_data = dev->data;
	const gpio_mtk_config_t *gpio_config = dev->config;

	if (line >= (gpio_config->idx * 32)) {
		line -= (gpio_config->idx * 32);

		gpio_fire_callbacks(&(gpio_data->gpio_callbacks), dev, BIT(line));
	}
}

int gpio_mtk_pin_configure(const struct device *dev, gpio_pin_t pin, gpio_flags_t flags)
{
	int ret;
	uint32_t shift = pin % 32;

	/* Set direction */
	if ((flags & GPIO_OUTPUT) != 0) {
		ret = mmio_write32(dev, REG_TYPE_DIR_SET, (uint32_t)(1 << shift));
	} else {
		ret = mmio_write32(dev, REG_TYPE_DIR_CLR, (uint32_t)(1 << shift));
	}

	if (ret != 0) {
		return ret;
	}

	/* Set output level */
	if ((flags & GPIO_OUTPUT) != 0) {
		if ((flags & GPIO_OUTPUT_INIT_HIGH) != 0) {
			ret = gpio_mtk_port_set_bits_raw(dev, (gpio_port_pins_t)(1 << shift));
		} else if ((flags & GPIO_OUTPUT_INIT_LOW) != 0) {
			ret = gpio_mtk_port_clr_bits_raw(dev, (gpio_port_pins_t)(1 << shift));
		}

		if (ret != 0) {
			return ret;
		}
	}

#ifdef GPIO_MTK_SUPPORT_PULL_UP_DOWN
	/* Set pull up / down */
	if ((flags & GPIO_PULL_UP) != 0) {
		ret = mmio_write32(dev, REG_TYPE_PULL_SEL_SET, (uint32_t)(1 << shift));
		if (ret == 0) {
			ret = mmio_write32(dev, REG_TYPE_PULL_EN_SET, (uint32_t)(1 << shift));
		}
	} else if ((flags & GPIO_PULL_DOWN) != 0) {
		ret = mmio_write32(dev, REG_TYPE_PULL_SEL_CLR, (uint32_t)(1 << shift));
		if (ret == 0) {
			ret = mmio_write32(dev, REG_TYPE_PULL_EN_SET, (uint32_t)(1 << shift));
		}
	} else {
		ret = mmio_write32(dev, REG_TYPE_PULL_EN_CLR, (uint32_t)(1 << shift));
	}
#endif

	return ret;
}

int gpio_mtk_port_get_raw(const struct device *dev, gpio_port_value_t *value)
{
	return mmio_read32(dev, REG_TYPE_DIN, value);
}

int gpio_mtk_port_set_masked_raw(const struct device *dev, gpio_port_pins_t mask,
				 gpio_port_value_t value)
{
	int ret;

	ret = gpio_mtk_port_set_bits_raw(dev, (value & mask));
	if (ret != 0) {
		return ret;
	}

	ret = gpio_mtk_port_clr_bits_raw(dev, ((value ^ mask) & mask));

	return ret;
}

int gpio_mtk_port_set_bits_raw(const struct device *dev, gpio_port_pins_t pins)
{
	int ret = 0;
	const gpio_mtk_config_t *gpio_config = dev->config;

	pins &= gpio_config->gpio_pin_mask;
	if (pins != 0) {
		ret = mmio_write32(dev, REG_TYPE_DOUT_SET, pins);
	}

	return ret;
}

int gpio_mtk_port_clr_bits_raw(const struct device *dev, gpio_port_pins_t pins)
{
	int ret = 0;
	const gpio_mtk_config_t *gpio_config = dev->config;

	pins &= gpio_config->gpio_pin_mask;
	if (pins != 0) {
		ret = mmio_write32(dev, REG_TYPE_DOUT_CLR, pins);
	}

	return ret;
}

int gpio_mtk_port_toggle_bits(const struct device *dev, gpio_port_pins_t pins)
{
	int ret;
	gpio_port_value_t value;

	ret = gpio_mtk_port_get_raw(dev, &value);
	if (ret != 0) {
		return ret;
	}

	ret = gpio_mtk_port_set_bits_raw(dev, ((value ^ pins) & pins));
	if (ret != 0) {
		return ret;
	}

	ret = gpio_mtk_port_clr_bits_raw(dev, (value & pins));

	return ret;
}

int gpio_mtk_pin_interrupt_configure(const struct device *dev, gpio_pin_t pin,
				     enum gpio_int_mode mode, enum gpio_int_trig trig)
{
	return 0;
}

int gpio_mtk_manage_callback(const struct device *dev, struct gpio_callback *callback, bool set)
{
	int ret;
	gpio_mtk_data_t *gpio_data = dev->data;

	ret = gpio_manage_callback(&(gpio_data->gpio_callbacks), callback, set);

	return ret;
}

int gpio_mtk_init(const struct device *dev)
{
	int ret;
	gpio_mtk_data_t *gpio_data = dev->data;
	const gpio_mtk_config_t *gpio_config = dev->config;

	DEVICE_MMIO_NAMED_MAP(dev, reg_base, K_MEM_CACHE_NONE);

	sys_slist_init(&(gpio_data->gpio_callbacks));

	/* Add the EINT driver callback. */
	ret = eint_mtk_init_callback(&(gpio_data->eint_callback), (gpio_config->idx * 32),
				     ((gpio_config->idx + 1) * 32), eint_handler, dev, NULL);
	if (ret != 0) {
		return ret;
	}

	ret = eint_mtk_add_callback(gpio_config->eint_dev, &(gpio_data->eint_callback));

	return ret;
}

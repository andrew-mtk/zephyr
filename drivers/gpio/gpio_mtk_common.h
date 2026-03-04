/*
 * Copyright (c) 2026 MediaTek
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef DRIVERS_GPIO_GPIO_MTK_COMMON_H_
#define DRIVERS_GPIO_GPIO_MTK_COMMON_H_

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/interrupt_controller/intc_mtk_eint.h>

#ifdef CONFIG_DT_HAS_MEDIATEK_MT8365_GPIO_ENABLED
#define GPIO_MTK_SUPPORT_PULL_UP_DOWN
#endif

#define INV_REG_OFFSET ((uint32_t)0xffffffff)

typedef enum {
	REG_TYPE_DIN = 0,
	REG_TYPE_DOUT_SET,
	REG_TYPE_DOUT_CLR,
	REG_TYPE_DIR_SET,
	REG_TYPE_DIR_CLR,
#ifdef GPIO_MTK_SUPPORT_PULL_UP_DOWN
	REG_TYPE_PULL_EN_SET,
	REG_TYPE_PULL_EN_CLR,
	REG_TYPE_PULL_SEL_SET,
	REG_TYPE_PULL_SEL_CLR,
#endif
} reg_type_t;

typedef uint32_t (*reg_offset_fct)(const struct device *dev, reg_type_t reg_type);

typedef struct {
	struct gpio_driver_config common;
	DEVICE_MMIO_NAMED_ROM(reg_base);
	const struct device *eint_dev;

	uint16_t idx;
	uint16_t num_gpio_pins;
	uint32_t gpio_pin_mask;

	reg_offset_fct reg_offset;
} gpio_mtk_config_t;

typedef struct {
	struct gpio_driver_data common;
	DEVICE_MMIO_NAMED_RAM(reg_base);
	eint_mtk_callback_t eint_callback;
	sys_slist_t gpio_callbacks;
} gpio_mtk_data_t;

#define DEV_CFG(dev)  ((const gpio_mtk_config_t *const)((dev)->config))
#define DEV_DATA(dev) ((gpio_mtk_data_t *const)((dev)->data))

extern int gpio_mtk_pin_configure(const struct device *dev, gpio_pin_t pin, gpio_flags_t flags);
extern int gpio_mtk_port_set_masked_raw(const struct device *dev, gpio_port_pins_t mask,
					gpio_port_value_t value);
extern int gpio_mtk_port_get_raw(const struct device *dev, gpio_port_value_t *value);
extern int gpio_mtk_port_set_bits_raw(const struct device *dev, gpio_port_pins_t pins);
extern int gpio_mtk_port_clr_bits_raw(const struct device *dev, gpio_port_pins_t pins);
extern int gpio_mtk_port_toggle_bits(const struct device *dev, gpio_port_pins_t pins);
extern int gpio_mtk_pin_interrupt_configure(const struct device *dev, gpio_pin_t pin,
					    enum gpio_int_mode mode, enum gpio_int_trig trig);
extern int gpio_mtk_manage_callback(const struct device *dev, struct gpio_callback *cb, bool set);
extern int gpio_mtk_init(const struct device *dev);

#endif /* DRIVERS_GPIO_GPIO_MTK_COMMON_H_ */

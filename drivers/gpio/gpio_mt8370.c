/*
 * Copyright (c) 2025 MediaTek
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT mediatek_mt8370_gpio

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/gpio/gpio_utils.h>
#include <zephyr/drivers/interrupt_controller/intc_mtk_eint.h>
#include <zephyr/drivers/pinctrl.h>
#include <zephyr/sys/slist.h>

#define GPIO_OFFSET_DIN_0      0x0200
#define GPIO_OFFSET_DIN_1      0x0210
#define GPIO_OFFSET_DIN_2      0x0220
#define GPIO_OFFSET_DIN_3      0x0230
#define GPIO_OFFSET_DIN_4      0x0240
#define GPIO_OFFSET_DIN_5      0x0250
#define GPIO_OFFSET_DOUT_0     0x0100
#define GPIO_OFFSET_DOUT_0_SET 0x0104
#define GPIO_OFFSET_DOUT_0_CLR 0x0108
#define GPIO_OFFSET_DOUT_1     0x0110
#define GPIO_OFFSET_DOUT_1_SET 0x0114
#define GPIO_OFFSET_DOUT_1_CLR 0x0118
#define GPIO_OFFSET_DOUT_2     0x0120
#define GPIO_OFFSET_DOUT_2_SET 0x0124
#define GPIO_OFFSET_DOUT_2_CLR 0x0128
#define GPIO_OFFSET_DOUT_3     0x0130
#define GPIO_OFFSET_DOUT_3_SET 0x0134
#define GPIO_OFFSET_DOUT_3_CLR 0x0138
#define GPIO_OFFSET_DOUT_4     0x0140
#define GPIO_OFFSET_DOUT_4_SET 0x0144
#define GPIO_OFFSET_DOUT_4_CLR 0x0148
#define GPIO_OFFSET_DOUT_5     0x0150
#define GPIO_OFFSET_DOUT_5_SET 0x0154
#define GPIO_OFFSET_DOUT_5_CLR 0x0158
#define GPIO_OFFSET_DIR_0      0x0000
#define GPIO_OFFSET_DIR_0_SET  0x0004
#define GPIO_OFFSET_DIR_0_CLR  0x0008
#define GPIO_OFFSET_DIR_1      0x0010
#define GPIO_OFFSET_DIR_1_SET  0x0014
#define GPIO_OFFSET_DIR_1_CLR  0x0018
#define GPIO_OFFSET_DIR_2      0x0020
#define GPIO_OFFSET_DIR_2_SET  0x0024
#define GPIO_OFFSET_DIR_2_CLR  0x0028
#define GPIO_OFFSET_DIR_3      0x0030
#define GPIO_OFFSET_DIR_3_SET  0x0034
#define GPIO_OFFSET_DIR_3_CLR  0x0038
#define GPIO_OFFSET_DIR_4      0x0040
#define GPIO_OFFSET_DIR_4_SET  0x0044
#define GPIO_OFFSET_DIR_4_CLR  0x0048
#define GPIO_OFFSET_DIR_5      0x0050
#define GPIO_OFFSET_DIR_5_SET  0x0054
#define GPIO_OFFSET_DIR_5_CLR  0x0058

// #define GPIO_OFFSET_PULL_EN_0			0x0860
// #define GPIO_OFFSET_PULL_EN_0_SET		0x0864
// #define GPIO_OFFSET_PULL_EN_0_CLR		0x0868
// #define GPIO_OFFSET_PULL_EN_1			0x0870
// #define GPIO_OFFSET_PULL_EN_1_SET		0x0874
// #define GPIO_OFFSET_PULL_EN_1_CLR		0x0878
// #define GPIO_OFFSET_PULL_EN_2			0x0880
// #define GPIO_OFFSET_PULL_EN_2_SET		0x0884
// #define GPIO_OFFSET_PULL_EN_2_CLR		0x0888
// #define GPIO_OFFSET_PULL_EN_3			0x0890
// #define GPIO_OFFSET_PULL_EN_3_SET		0x0894
// #define GPIO_OFFSET_PULL_EN_3_CLR		0x0898
// #define GPIO_OFFSET_PULL_EN_4			0x08a0
// #define GPIO_OFFSET_PULL_EN_4_SET		0x08a4
// #define GPIO_OFFSET_PULL_EN_4_CLR		0x08a8
// #define GPIO_OFFSET_PULL_SEL_0			0x0900
// #define GPIO_OFFSET_PULL_SEL_0_SET		0x0904
// #define GPIO_OFFSET_PULL_SEL_0_CLR		0x0908
// #define GPIO_OFFSET_PULL_SEL_1			0x0910
// #define GPIO_OFFSET_PULL_SEL_1_SET		0x0914
// #define GPIO_OFFSET_PULL_SEL_1_CLR		0x0918
// #define GPIO_OFFSET_PULL_SEL_2			0x0920
// #define GPIO_OFFSET_PULL_SEL_2_SET		0x0924
// #define GPIO_OFFSET_PULL_SEL_2_CLR		0x0928
// #define GPIO_OFFSET_PULL_SEL_3			0x0930
// #define GPIO_OFFSET_PULL_SEL_3_SET		0x0934
// #define GPIO_OFFSET_PULL_SEL_3_CLR		0x0938
// #define GPIO_OFFSET_PULL_SEL_4			0x0940
// #define GPIO_OFFSET_PULL_SEL_4_SET		0x0944
// #define GPIO_OFFSET_PULL_SEL_4_CLR		0x0948

static const uint32_t pin_to_din_offset_map[] = {
	/*   0 -  31 */ GPIO_OFFSET_DIN_0,
	/*  32 -  63 */ GPIO_OFFSET_DIN_1,
	/*  64 -  95 */ GPIO_OFFSET_DIN_2,
	/*  96 - 127 */ GPIO_OFFSET_DIN_3,
	/* 128 - 159 */ GPIO_OFFSET_DIN_4,
	/* 160 - 176 */ GPIO_OFFSET_DIN_5,
	/* 177 - 191 Reserved */
};

static const uint32_t pin_to_dout_set_offset_map[] = {
	/*   0 -  31 */ GPIO_OFFSET_DOUT_0_SET,
	/*  32 -  63 */ GPIO_OFFSET_DOUT_1_SET,
	/*  64 -  95 */ GPIO_OFFSET_DOUT_2_SET,
	/*  96 - 127 */ GPIO_OFFSET_DOUT_3_SET,
	/* 128 - 159 */ GPIO_OFFSET_DOUT_4_SET,
	/* 160 - 176 */ GPIO_OFFSET_DOUT_5_SET,
};

static const uint32_t pin_to_dout_clr_offset_map[] = {
	/*   0 -  31 */ GPIO_OFFSET_DOUT_0_CLR,
	/*  32 -  63 */ GPIO_OFFSET_DOUT_1_CLR,
	/*  64 -  95 */ GPIO_OFFSET_DOUT_2_CLR,
	/*  96 - 127 */ GPIO_OFFSET_DOUT_3_CLR,
	/* 128 - 159 */ GPIO_OFFSET_DOUT_4_CLR,
	/* 160 - 176 */ GPIO_OFFSET_DOUT_5_CLR,
};

static const uint32_t pin_to_dir_set_offset_map[] = {
	/*   0 -  31 */ GPIO_OFFSET_DIR_0_SET,
	/*  32 -  63 */ GPIO_OFFSET_DIR_1_SET,
	/*  64 -  95 */ GPIO_OFFSET_DIR_2_SET,
	/*  96 - 127 */ GPIO_OFFSET_DIR_3_SET,
	/* 128 - 159 */ GPIO_OFFSET_DIR_4_SET,
	/* 160 - 176 */ GPIO_OFFSET_DIR_5_SET,
};

static const uint32_t pin_to_dir_clr_offset_map[] = {
	/*   0 -  31 */ GPIO_OFFSET_DIR_0_CLR,
	/*  32 -  63 */ GPIO_OFFSET_DIR_1_CLR,
	/*  64 -  95 */ GPIO_OFFSET_DIR_2_CLR,
	/*  96 - 127 */ GPIO_OFFSET_DIR_3_CLR,
	/* 128 - 159 */ GPIO_OFFSET_DIR_4_CLR,
	/* 160 - 176 */ GPIO_OFFSET_DIR_5_CLR,
};

// static const uint32_t pin_to_pull_en_set_offset_map[] = {
// 	/*   0 -  31 */ GPIO_OFFSET_PULL_EN_0_SET,
// 	/*  32 -  63 */ GPIO_OFFSET_PULL_EN_1_SET,
// 	/*  64 -  95 */ GPIO_OFFSET_PULL_EN_2_SET,
// 	/*  96 - 127 */ GPIO_OFFSET_PULL_EN_3_SET,
// 	/* 128 - 159 */ GPIO_OFFSET_PULL_EN_4_SET,
// };

// static const uint32_t pin_to_pull_en_clr_offset_map[] = {
// 	/*   0 -  31 */ GPIO_OFFSET_PULL_EN_0_CLR,
// 	/*  32 -  63 */ GPIO_OFFSET_PULL_EN_1_CLR,
// 	/*  64 -  95 */ GPIO_OFFSET_PULL_EN_2_CLR,
// 	/*  96 - 127 */ GPIO_OFFSET_PULL_EN_3_CLR,
// 	/* 128 - 159 */ GPIO_OFFSET_PULL_EN_4_CLR,
// };

// static const uint32_t pin_to_pull_sel_set_offset_map[] = {
// 	/*   0 -  31 */ GPIO_OFFSET_PULL_SEL_0_SET,
// 	/*  32 -  63 */ GPIO_OFFSET_PULL_SEL_1_SET,
// 	/*  64 -  95 */ GPIO_OFFSET_PULL_SEL_2_SET,
// 	/*  96 - 127 */ GPIO_OFFSET_PULL_SEL_3_SET,
// 	/* 128 - 159 */ GPIO_OFFSET_PULL_SEL_4_SET,
// };

// static const uint32_t pin_to_pull_sel_clr_offset_map[] = {
// 	/*   0 -  31 */ GPIO_OFFSET_PULL_SEL_0_CLR,
// 	/*  32 -  63 */ GPIO_OFFSET_PULL_SEL_1_CLR,
// 	/*  64 -  95 */ GPIO_OFFSET_PULL_SEL_2_CLR,
// 	/*  96 - 127 */ GPIO_OFFSET_PULL_SEL_3_CLR,
// 	/* 128 - 159 */ GPIO_OFFSET_PULL_SEL_4_CLR,
// };

#define DEV_CFG(dev)  ((const gpio_mtk_config_t *const)((dev)->config))
#define DEV_DATA(dev) ((gpio_mtk_data_t *const)((dev)->data))

typedef struct {
	struct gpio_driver_config common;
	DEVICE_MMIO_NAMED_ROM(reg_base);
	const struct device *eint_dev;
	uint16_t idx;
	uint16_t num_gpio_pins;
	uint32_t gpio_pin_mask;
} gpio_mtk_config_t;

typedef struct {
	struct gpio_driver_data common;
	DEVICE_MMIO_NAMED_RAM(reg_base);
	eint_mtk_callback_t eint_callback;
	sys_slist_t gpio_callbacks;
} gpio_mtk_data_t;

static int gpio_mtk_pin_configure(const struct device *dev, gpio_pin_t pin, gpio_flags_t flags);
static int gpio_mtk_port_set_masked_raw(const struct device *dev, gpio_port_pins_t mask,
					gpio_port_value_t value);
static int gpio_mtk_port_get_raw(const struct device *dev, gpio_port_value_t *value);
static int gpio_mtk_port_set_bits_raw(const struct device *dev, gpio_port_pins_t pins);
static int gpio_mtk_port_clr_bits_raw(const struct device *dev, gpio_port_pins_t pins);
static int gpio_mtk_port_toggle_bits(const struct device *dev, gpio_port_pins_t pins);
static int gpio_mtk_pin_interrupt_configure(const struct device *dev, gpio_pin_t pin,
					    enum gpio_int_mode mode, enum gpio_int_trig trig);
static int gpio_mtk_manage_callback(const struct device *dev, struct gpio_callback *cb, bool set);
static void eint_handler(const struct device *dev, uint8_t line, void *arg);
static int gpio_mtk_init(const struct device *dev);

static int gpio_mtk_pin_configure(const struct device *dev, gpio_pin_t pin, gpio_flags_t flags)
{
	uint32_t shift = pin % 32;
	const gpio_mtk_config_t *gpio_config = dev->config;

	/* Set direction */
	if ((flags & GPIO_OUTPUT) != 0) {
		sys_write32((1 << shift), DEVICE_MMIO_NAMED_GET(dev, reg_base) +
						  pin_to_dir_set_offset_map[gpio_config->idx]);
	} else {
		sys_write32((1 << shift), DEVICE_MMIO_NAMED_GET(dev, reg_base) +
						  pin_to_dir_clr_offset_map[gpio_config->idx]);
	}

	/* Set output level */
	if ((flags & GPIO_OUTPUT) != 0) {
		if ((flags & GPIO_OUTPUT_INIT_HIGH) != 0) {
			gpio_mtk_port_set_bits_raw(dev, (gpio_port_pins_t)(1 << shift));
		} else if ((flags & GPIO_OUTPUT_INIT_LOW) != 0) {
			gpio_mtk_port_clr_bits_raw(dev, (gpio_port_pins_t)(1 << shift));
		}
	}

	/* Set pull up / down */
	// if ((flags & GPIO_PULL_UP) != 0) {
	// 	sys_write32((1 << shift), DEVICE_MMIO_NAMED_GET(dev, reg_base) +
	// 					  pin_to_pull_sel_set_offset_map[gpio_config->idx]);
	// 	sys_write32((1 << shift), DEVICE_MMIO_NAMED_GET(dev, reg_base) +
	// 					  pin_to_pull_en_set_offset_map[gpio_config->idx]);
	// } else if ((flags & GPIO_PULL_DOWN) != 0) {
	// 	sys_write32((1 << shift), DEVICE_MMIO_NAMED_GET(dev, reg_base) +
	// 					  pin_to_pull_sel_clr_offset_map[gpio_config->idx]);
	// 	sys_write32((1 << shift), DEVICE_MMIO_NAMED_GET(dev, reg_base) +
	// 					  pin_to_pull_en_set_offset_map[gpio_config->idx]);
	// } else {
	// 	sys_write32((1 << shift), DEVICE_MMIO_NAMED_GET(dev, reg_base) +
	// 					  pin_to_pull_en_clr_offset_map[gpio_config->idx]);
	// }

	return 0;
}

static int gpio_mtk_port_get_raw(const struct device *dev, gpio_port_value_t *value)
{
	const gpio_mtk_config_t *gpio_config = dev->config;

	*value = sys_read32(DEVICE_MMIO_NAMED_GET(dev, reg_base) +
			    pin_to_din_offset_map[gpio_config->idx]) &
		 gpio_config->gpio_pin_mask;

	return 0;
}

static int gpio_mtk_port_set_masked_raw(const struct device *dev, gpio_port_pins_t mask,
					gpio_port_value_t value)
{
	gpio_mtk_port_set_bits_raw(dev, (value & mask));
	gpio_mtk_port_clr_bits_raw(dev, ((value ^ mask) & mask));

	return 0;
}

static int gpio_mtk_port_set_bits_raw(const struct device *dev, gpio_port_pins_t pins)
{
	const gpio_mtk_config_t *gpio_config = dev->config;

	pins &= gpio_config->gpio_pin_mask;
	if (pins != 0) {
		sys_write32(pins, DEVICE_MMIO_NAMED_GET(dev, reg_base) +
					  pin_to_dout_set_offset_map[gpio_config->idx]);
	}

	return 0;
}

static int gpio_mtk_port_clr_bits_raw(const struct device *dev, gpio_port_pins_t pins)
{
	const gpio_mtk_config_t *gpio_config = dev->config;

	pins &= gpio_config->gpio_pin_mask;
	if (pins != 0) {
		sys_write32(pins, DEVICE_MMIO_NAMED_GET(dev, reg_base) +
					  pin_to_dout_clr_offset_map[gpio_config->idx]);
	}

	return 0;
}

static int gpio_mtk_port_toggle_bits(const struct device *dev, gpio_port_pins_t pins)
{
	gpio_port_value_t value;

	gpio_mtk_port_get_raw(dev, &value);

	gpio_mtk_port_set_bits_raw(dev, ((value ^ pins) & pins));
	gpio_mtk_port_clr_bits_raw(dev, (value & pins));

	return 0;
}

static int gpio_mtk_pin_interrupt_configure(const struct device *dev, gpio_pin_t pin,
					    enum gpio_int_mode mode, enum gpio_int_trig trig)
{
	return 0;
}

static int gpio_mtk_manage_callback(const struct device *dev, struct gpio_callback *callback,
				    bool set)
{
	int ret;
	gpio_mtk_data_t *gpio_data = dev->data;

	ret = gpio_manage_callback(&(gpio_data->gpio_callbacks), callback, set);

	return ret;
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

static int gpio_mtk_init(const struct device *dev)
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
	if (ret != 0) {
		return ret;
	}

	return 0;
}

static DEVICE_API(gpio, gpio_mtk_driver_api) = {
	.pin_configure = gpio_mtk_pin_configure,
	.port_get_raw = gpio_mtk_port_get_raw,
	.port_set_masked_raw = gpio_mtk_port_set_masked_raw,
	.port_set_bits_raw = gpio_mtk_port_set_bits_raw,
	.port_clear_bits_raw = gpio_mtk_port_clr_bits_raw,
	.port_toggle_bits = gpio_mtk_port_toggle_bits,
	.pin_interrupt_configure = gpio_mtk_pin_interrupt_configure,
	.manage_callback = gpio_mtk_manage_callback,
};

#define GPIO_DECLARE_CONFIG(n)                                                                     \
	static const gpio_mtk_config_t gpio_mtk_##n##_config = {                                   \
		.common = {.port_pin_mask = GPIO_PORT_PIN_MASK_FROM_DT_INST(0)},                   \
		DEVICE_MMIO_NAMED_ROM_INIT(reg_base, DT_INST_PARENT(n)),                           \
		.eint_dev = &DEVICE_DT_NAME_GET(DT_INST_PROP(n, interrupt_parent)),                \
		.idx = DT_INST_REG_ADDR(n),                                                        \
		.num_gpio_pins = DT_INST_PROP(n, ngpios),                                          \
		.gpio_pin_mask = GPIO_PORT_PIN_MASK_FROM_NGPIOS(DT_INST_PROP(n, ngpios)),          \
	};

#define GPIO_INIT(n)                                                                               \
	static gpio_mtk_data_t gpio_mtk_##n##_data;                                                \
	static const gpio_mtk_config_t gpio_mtk_##n##_config;                                      \
	GPIO_DECLARE_CONFIG(n)                                                                     \
	DEVICE_DT_INST_DEFINE(n, &gpio_mtk_init, NULL, &gpio_mtk_##n##_data,                       \
			      &gpio_mtk_##n##_config, PRE_KERNEL_1, CONFIG_GPIO_INIT_PRIORITY,     \
			      &gpio_mtk_driver_api);

DT_INST_FOREACH_STATUS_OKAY(GPIO_INIT)

/*
 * Copyright (c) 2025 MediaTek
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT mediatek_mt8365_gpio

#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio/gpio_utils.h>

#include "gpio_mtk_common.h"

/* Register offsets in order of offset values. */
#define GPIO_OFFSET_DIN_0          0x0000
#define GPIO_OFFSET_DIN_1          0x0010
#define GPIO_OFFSET_DIN_2          0x0020
#define GPIO_OFFSET_DIN_3          0x0030
#define GPIO_OFFSET_DIN_4          0x0040
#define GPIO_OFFSET_DOUT_0         0x00a0
#define GPIO_OFFSET_DOUT_0_SET     0x00a4
#define GPIO_OFFSET_DOUT_0_CLR     0x00a8
#define GPIO_OFFSET_DOUT_1         0x00b0
#define GPIO_OFFSET_DOUT_1_SET     0x00b4
#define GPIO_OFFSET_DOUT_1_CLR     0x00b8
#define GPIO_OFFSET_DOUT_2         0x00c0
#define GPIO_OFFSET_DOUT_2_SET     0x00c4
#define GPIO_OFFSET_DOUT_2_CLR     0x00c8
#define GPIO_OFFSET_DOUT_3         0x00d0
#define GPIO_OFFSET_DOUT_3_SET     0x00d4
#define GPIO_OFFSET_DOUT_3_CLR     0x00d8
#define GPIO_OFFSET_DOUT_4         0x00e0
#define GPIO_OFFSET_DOUT_4_SET     0x00e4
#define GPIO_OFFSET_DOUT_4_CLR     0x00e8
#define GPIO_OFFSET_DIR_0          0x0140
#define GPIO_OFFSET_DIR_0_SET      0x0144
#define GPIO_OFFSET_DIR_0_CLR      0x0148
#define GPIO_OFFSET_DIR_1          0x0150
#define GPIO_OFFSET_DIR_1_SET      0x0154
#define GPIO_OFFSET_DIR_1_CLR      0x0158
#define GPIO_OFFSET_DIR_2          0x0160
#define GPIO_OFFSET_DIR_2_SET      0x0164
#define GPIO_OFFSET_DIR_2_CLR      0x0168
#define GPIO_OFFSET_DIR_3          0x0170
#define GPIO_OFFSET_DIR_3_SET      0x0174
#define GPIO_OFFSET_DIR_3_CLR      0x0178
#define GPIO_OFFSET_DIR_4          0x0180
#define GPIO_OFFSET_DIR_4_SET      0x0184
#define GPIO_OFFSET_DIR_4_CLR      0x0188
#define GPIO_OFFSET_PULL_EN_0      0x0860
#define GPIO_OFFSET_PULL_EN_0_SET  0x0864
#define GPIO_OFFSET_PULL_EN_0_CLR  0x0868
#define GPIO_OFFSET_PULL_EN_1      0x0870
#define GPIO_OFFSET_PULL_EN_1_SET  0x0874
#define GPIO_OFFSET_PULL_EN_1_CLR  0x0878
#define GPIO_OFFSET_PULL_EN_2      0x0880
#define GPIO_OFFSET_PULL_EN_2_SET  0x0884
#define GPIO_OFFSET_PULL_EN_2_CLR  0x0888
#define GPIO_OFFSET_PULL_EN_3      0x0890
#define GPIO_OFFSET_PULL_EN_3_SET  0x0894
#define GPIO_OFFSET_PULL_EN_3_CLR  0x0898
#define GPIO_OFFSET_PULL_EN_4      0x08a0
#define GPIO_OFFSET_PULL_EN_4_SET  0x08a4
#define GPIO_OFFSET_PULL_EN_4_CLR  0x08a8
#define GPIO_OFFSET_PULL_SEL_0     0x0900
#define GPIO_OFFSET_PULL_SEL_0_SET 0x0904
#define GPIO_OFFSET_PULL_SEL_0_CLR 0x0908
#define GPIO_OFFSET_PULL_SEL_1     0x0910
#define GPIO_OFFSET_PULL_SEL_1_SET 0x0914
#define GPIO_OFFSET_PULL_SEL_1_CLR 0x0918
#define GPIO_OFFSET_PULL_SEL_2     0x0920
#define GPIO_OFFSET_PULL_SEL_2_SET 0x0924
#define GPIO_OFFSET_PULL_SEL_2_CLR 0x0928
#define GPIO_OFFSET_PULL_SEL_3     0x0930
#define GPIO_OFFSET_PULL_SEL_3_SET 0x0934
#define GPIO_OFFSET_PULL_SEL_3_CLR 0x0938
#define GPIO_OFFSET_PULL_SEL_4     0x0940
#define GPIO_OFFSET_PULL_SEL_4_SET 0x0944
#define GPIO_OFFSET_PULL_SEL_4_CLR 0x0948

#define GPIO_OFFSET_DIN_DELTA          (GPIO_OFFSET_DIN_1 - GPIO_OFFSET_DIN_0)
#define GPIO_OFFSET_DOUT_SET_DELTA     (GPIO_OFFSET_DOUT_1_SET - GPIO_OFFSET_DOUT_0_SET)
#define GPIO_OFFSET_DOUT_CLR_DELTA     (GPIO_OFFSET_DOUT_1_CLR - GPIO_OFFSET_DOUT_0_CLR)
#define GPIO_OFFSET_DIR_SET_DELTA      (GPIO_OFFSET_DIR_1_SET - GPIO_OFFSET_DIR_0_SET)
#define GPIO_OFFSET_DIR_CLR_DELTA      (GPIO_OFFSET_DIR_1_CLR - GPIO_OFFSET_DIR_0_CLR)
#define GPIO_OFFSET_PULL_EN_SET_DELTA  (GPIO_OFFSET_PULL_EN_1_SET - GPIO_OFFSET_PULL_EN_0_SET)
#define GPIO_OFFSET_PULL_EN_CLR_DELTA  (GPIO_OFFSET_PULL_EN_1_CLR - GPIO_OFFSET_PULL_EN_0_CLR)
#define GPIO_OFFSET_PULL_SEL_SET_DELTA (GPIO_OFFSET_PULL_SEL_1_SET - GPIO_OFFSET_PULL_SEL_0_SET)
#define GPIO_OFFSET_PULL_SEL_CLR_DELTA (GPIO_OFFSET_PULL_SEL_1_CLR - GPIO_OFFSET_PULL_SEL_0_CLR)

static uint32_t reg_offset(const struct device *dev, reg_type_t reg_type);

static uint32_t reg_offset(const struct device *dev, reg_type_t reg_type)
{
	const gpio_mtk_config_t *gpio_config = dev->config;

	switch (reg_type) {
	case REG_TYPE_DIN:
		return (uint32_t)(GPIO_OFFSET_DIN_0 + (gpio_config->idx * GPIO_OFFSET_DIN_DELTA));
		break;

	case REG_TYPE_DOUT_SET:
		return (uint32_t)(GPIO_OFFSET_DOUT_0_SET +
				  (gpio_config->idx * GPIO_OFFSET_DOUT_SET_DELTA));
		break;

	case REG_TYPE_DOUT_CLR:
		return (uint32_t)(GPIO_OFFSET_DOUT_0_CLR +
				  (gpio_config->idx * GPIO_OFFSET_DOUT_CLR_DELTA));
		break;

	case REG_TYPE_DIR_SET:
		return (uint32_t)(GPIO_OFFSET_DIR_0_SET +
				  (gpio_config->idx * GPIO_OFFSET_DIR_SET_DELTA));
		break;

	case REG_TYPE_DIR_CLR:
		return (uint32_t)(GPIO_OFFSET_DIR_0_CLR +
				  (gpio_config->idx * GPIO_OFFSET_DIR_CLR_DELTA));
		break;

	case REG_TYPE_PULL_EN_SET:
		return (uint32_t)(GPIO_OFFSET_PULL_EN_0_SET +
				  (gpio_config->idx * GPIO_OFFSET_PULL_EN_SET_DELTA));
		break;

	case REG_TYPE_PULL_EN_CLR:
		return (uint32_t)(GPIO_OFFSET_PULL_EN_0_CLR +
				  (gpio_config->idx * GPIO_OFFSET_PULL_EN_CLR_DELTA));
		break;

	case REG_TYPE_PULL_SEL_SET:
		return (uint32_t)(GPIO_OFFSET_PULL_SEL_0_SET +
				  (gpio_config->idx * GPIO_OFFSET_PULL_SEL_SET_DELTA));
		break;

	case REG_TYPE_PULL_SEL_CLR:
		return (uint32_t)(GPIO_OFFSET_PULL_SEL_0_CLR +
				  (gpio_config->idx * GPIO_OFFSET_PULL_SEL_CLR_DELTA));
		break;

	default:
		break;
	}

	return INV_REG_OFFSET;
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

#define GPIO_DECLARE_CFG(n)                                                                        \
	static gpio_mtk_data_t gpio_mtk_##n##_data;                                                \
                                                                                                   \
	static const gpio_mtk_config_t gpio_mtk_##n##_config = {                                   \
		.common = {.port_pin_mask = GPIO_PORT_PIN_MASK_FROM_DT_INST(0)},                   \
		DEVICE_MMIO_NAMED_ROM_INIT(reg_base, DT_INST_PARENT(n)),                           \
		.eint_dev = &DEVICE_DT_NAME_GET(DT_INST_PROP(n, interrupt_parent)),                \
		.idx = DT_INST_REG_ADDR(n),                                                        \
		.num_gpio_pins = DT_INST_PROP(n, ngpios),                                          \
		.gpio_pin_mask = GPIO_PORT_PIN_MASK_FROM_NGPIOS(DT_INST_PROP(n, ngpios)),          \
		.reg_offset = reg_offset,                                                          \
	};

#define GPIO_INIT(n)                                                                               \
	GPIO_DECLARE_CFG(n)                                                                        \
	DEVICE_DT_INST_DEFINE(n, &gpio_mtk_init, NULL, &gpio_mtk_##n##_data,                       \
			      &gpio_mtk_##n##_config, PRE_KERNEL_1, CONFIG_GPIO_INIT_PRIORITY,     \
			      &gpio_mtk_driver_api);

DT_INST_FOREACH_STATUS_OKAY(GPIO_INIT)

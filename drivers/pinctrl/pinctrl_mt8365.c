/*
 * Copyright (c) 2025 MediaTek
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT mediatek_mt8365_pinctrl

#include <zephyr/arch/cpu.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/pinctrl.h>

#include "pinctrl_mtk_common.h"

#define PINCTRL_BASE_ADDR DT_INST_REG_ADDR(0)

#define PINCTRL_OFFSET_MODE_0 0x01e0
#define PINCTRL_OFFSET_MODE_1 0x01f0
#define PINCTRL_OFFSET_MODE_2 0x0200
#define PINCTRL_OFFSET_MODE_3 0x0210
#define PINCTRL_OFFSET_MODE_4 0x0220
#define PINCTRL_OFFSET_MODE_5 0x0230
#define PINCTRL_OFFSET_MODE_6 0x0240
#define PINCTRL_OFFSET_MODE_7 0x0250
#define PINCTRL_OFFSET_MODE_8 0x0260
#define PINCTRL_OFFSET_MODE_9 0x0270
#define PINCTRL_OFFSET_MODE_A 0x0280
#define PINCTRL_OFFSET_MODE_B 0x0290
#define PINCTRL_OFFSET_MODE_C 0x02a0
#define PINCTRL_OFFSET_MODE_D 0x02b0
#define PINCTRL_OFFSET_MODE_E 0x02c0

static const pinctrl_mtk_config_t pinctrl_mtk_config = {
	.max_pin = 145,
	.max_func = 8,
};

static const uint32_t pin_to_mode_offset_map[] = {
	/*   0 -   9 */ PINCTRL_OFFSET_MODE_0,
	/*  10 -  19 */ PINCTRL_OFFSET_MODE_1,
	/*  20 -  29 */ PINCTRL_OFFSET_MODE_2,
	/*  30 -  39 */ PINCTRL_OFFSET_MODE_3,
	/*  40 -  49 */ PINCTRL_OFFSET_MODE_4,
	/*  50 -  59 */ PINCTRL_OFFSET_MODE_5,
	/*  60 -  69 */ PINCTRL_OFFSET_MODE_6,
	/*  70 -  79 */ PINCTRL_OFFSET_MODE_7,
	/*  80 -  89 */ PINCTRL_OFFSET_MODE_8,
	/*  90 -  99 */ PINCTRL_OFFSET_MODE_9,
	/* 100 - 109 */ PINCTRL_OFFSET_MODE_A,
	/* 110 - 119 */ PINCTRL_OFFSET_MODE_B,
	/* 120 - 129 */ PINCTRL_OFFSET_MODE_C,
	/* 130 - 139 */ PINCTRL_OFFSET_MODE_D,
	/* 140 - 149 */ PINCTRL_OFFSET_MODE_E,
};

static uint32_t to_mode_offset(uint16_t pin);
static uint32_t to_mode_shift(uint16_t pin);
static uint32_t to_mode_mask(uint16_t pin);

static uint32_t to_mode_offset(uint16_t pin)
{
	/* Each 32 bit MODE register controls 10 pins. */
	return (pin_to_mode_offset_map[pin / 10]);
}

static uint32_t to_mode_shift(uint16_t pin)
{
	return ((pin % 10) * 3);
}

static uint32_t to_mode_mask(uint16_t pin)
{
	return (0x00000003 << to_mode_shift(pin));
}

static int pinctrl_set_func(uint16_t pin, uint16_t func)
{
	uint32_t val;
	uint32_t offset;

	if ((pin >= pinctrl_mtk_config.max_pin) || (func >= pinctrl_mtk_config.max_func)) {
		return -EINVAL;
	}

	offset = to_mode_offset(pin);

	/* Read the existing function value and replace */
	/* with the new function value.                 */
	val = sys_read32(PINCTRL_BASE_ADDR + offset);
	val &= ~(to_mode_mask(pin));
	val |= ((uint32_t)func) << to_mode_shift(pin);

	sys_write32(val, (PINCTRL_BASE_ADDR + offset));

	return 0;
}

int pinctrl_configure_pins(const pinctrl_soc_pin_t *pins, uint8_t pin_cnt, uintptr_t reg)
{
	while (pin_cnt > 0) {
		int ret;

		ret = pinctrl_set_func(pins->pin, pins->func);
		if (ret != 0) {
			return ret;
		}

		pins++;
		pin_cnt--;
	}

	return 0;
}

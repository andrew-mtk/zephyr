/*
 * Copyright (c) 2025 MediaTek
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT mediatek_mt8390_pinctrl

#include <zephyr/arch/cpu.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/pinctrl.h>

#define PINCTRL_BASE_ADDR DT_INST_REG_ADDR(0)

// #define PINCTRL_OFFSET_EH_CF				0x0050
// #define PINCTRL_OFFSET_RESL_CFG	            0x0060
// #define PINCTRL_OFFSET_PUPU_CFG_0			0x0070
// #define PINCTRL_OFFSET_PUPU_CFG_0_SET		0x0074
// #define PINCTRL_OFFSET_PUPU_CFG_0_CLR		0x0078
// #define PINCTRL_OFFSET_PUPU_CFG_1			0x0080
// #define PINCTRL_OFFSET_PUPU_CFG_1_SET		0x0084
// #define PINCTRL_OFFSET_PUPU_CFG_1_CLR		0x0088
// #define PINCTRL_OFFSET_PUPU_CFG_2			0x0090
// #define PINCTRL_OFFSET_PUPU_CFG_2_SET		0x0094
// #define PINCTRL_OFFSET_PUPU_CFG_2_CLR		0x0098
// #define PINCTRL_OFFSET_DOUT_0				0x00a0
// #define PINCTRL_OFFSET_DOUT_0_SET			0x00a4
// #define PINCTRL_OFFSET_DOUT_0_CLR			0x00a8
// #define PINCTRL_OFFSET_DOUT_1				0x00b0
// #define PINCTRL_OFFSET_DOUT_1_SET			0x00b4
// #define PINCTRL_OFFSET_DOUT_1_CLR			0x00b8
// #define PINCTRL_OFFSET_DOUT_2				0x00c0
// #define PINCTRL_OFFSET_DOUT_2_SET			0x00c4
// #define PINCTRL_OFFSET_DOUT_2_CLR			0x00c8
// #define PINCTRL_OFFSET_DOUT_3				0x00d0
// #define PINCTRL_OFFSET_DOUT_3_SET			0x00d4
// #define PINCTRL_OFFSET_DOUT_3_CLR			0x00d8
// #define PINCTRL_OFFSET_DOUT_4				0x00e0
// #define PINCTRL_OFFSET_DOUT_4_SET			0x00e4
// #define PINCTRL_OFFSET_DOUT_4_CLR			0x00e8
// #define PINCTRL_OFFSET_PUPU_CFG_3			0x00f0
// #define PINCTRL_OFFSET_PUPU_CFG_3_SET		0x00f4
// #define PINCTRL_OFFSET_PUPU_CFG_3_CLR		0x00f8
// #define PINCTRL_OFFSET_DEBUG_MON_SEL		0x0100
// #define PINCTRL_OFFSET_DIR_0				0x0140
// #define PINCTRL_OFFSET_DIR_0_SET			0x0144
// #define PINCTRL_OFFSET_DIR_0_CLR			0x0148
// #define PINCTRL_OFFSET_DIR_1				0x0150
// #define PINCTRL_OFFSET_DIR_1_SET			0x0154
// #define PINCTRL_OFFSET_DIR_1_CLR			0x0158
// #define PINCTRL_OFFSET_DIR_2				0x0160
// #define PINCTRL_OFFSET_DIR_2_SET			0x0164
// #define PINCTRL_OFFSET_DIR_2_CLR			0x0168
// #define PINCTRL_OFFSET_DIR_3				0x0170
// #define PINCTRL_OFFSET_DIR_3_SET			0x0174
// #define PINCTRL_OFFSET_DIR_3_CLR			0x0178
// #define PINCTRL_OFFSET_DIR_4				0x0180
// #define PINCTRL_OFFSET_DIR_4_SET			0x0184
// #define PINCTRL_OFFSET_DIR_4_CLR			0x0188

#define PINCTRL_OFFSET_MODE_0  0x0300
#define PINCTRL_OFFSET_MODE_1  0x0310
#define PINCTRL_OFFSET_MODE_2  0x0320
#define PINCTRL_OFFSET_MODE_3  0x0330
#define PINCTRL_OFFSET_MODE_4  0x0340
#define PINCTRL_OFFSET_MODE_5  0x0350
#define PINCTRL_OFFSET_MODE_6  0x0360
#define PINCTRL_OFFSET_MODE_7  0x0370
#define PINCTRL_OFFSET_MODE_8  0x0380
#define PINCTRL_OFFSET_MODE_9  0x0390
#define PINCTRL_OFFSET_MODE_10 0x03a0
#define PINCTRL_OFFSET_MODE_11 0x03b0
#define PINCTRL_OFFSET_MODE_12 0x03c0
#define PINCTRL_OFFSET_MODE_13 0x03d0
#define PINCTRL_OFFSET_MODE_14 0x03e0
#define PINCTRL_OFFSET_MODE_15 0x03f0
#define PINCTRL_OFFSET_MODE_16 0x0400
#define PINCTRL_OFFSET_MODE_17 0x0410
#define PINCTRL_OFFSET_MODE_18 0x0420
#define PINCTRL_OFFSET_MODE_19 0x0430
#define PINCTRL_OFFSET_MODE_20 0x0440
#define PINCTRL_OFFSET_MODE_21 0x0450
#define PINCTRL_OFFSET_MODE_22 0x0460

// #define PINCTRL_OFFSET_IES_CFG_0			0x0410
// #define PINCTRL_OFFSET_IES_CFG_1			0x0420
// #define PINCTRL_OFFSET_PINCTRL_BANK         0x0430
// #define PINCTRL_OFFSET_PINCTRL_TM           0x0440
// #define PINCTRL_OFFSET_MISC_CFG	            0x0450
// #define PINCTRL_OFFSET_MODE_CFG             0x0460
// #define PINCTRL_OFFSET_SMT_CFG_0			0x0470
// #define PINCTRL_OFFSET_SMT_CFG_1			0x0480
// #define PINCTRL_OFFSET_TDSEL_CFG_0			0x0510
// #define PINCTRL_OFFSET_TDSEL_CFG_2			0x0520
// #define PINCTRL_OFFSET_TDSEL_CFG_3			0x0530
// #define PINCTRL_OFFSET_TDSEL_CFG_4			0x0540
// #define PINCTRL_OFFSET_TDSEL_CFG_5			0x0550
// #define PINCTRL_OFFSET_TDSEL_CFG_6			0x0560
// #define PINCTRL_OFFSET_TDSEL_CFG_1			0x05a0
// #define PINCTRL_OFFSET_RDSEL_CFG_0			0x0610
// #define PINCTRL_OFFSET_RDSEL_CFG_1			0x0620
// #define PINCTRL_OFFSET_RDSEL_CFG_2			0x0630
// #define PINCTRL_OFFSET_RDSEL_CFG_3			0x0640
// #define PINCTRL_OFFSET_RDSEL_CFG_4			0x0650
// #define PINCTRL_OFFSET_RDSEL_CFG_5			0x0660
// #define PINCTRL_OFFSET_RDSEL_CFG_6			0x0670
// #define PINCTRL_OFFSET_RDSEL_CFG_7			0x0680
// #define PINCTRL_OFFSET_RDSEL_CFG_8			0x0690
// #define PINCTRL_OFFSET_RDSEL_CFG_9			0x06a0
// #define PINCTRL_OFFSET_DRV_CFG_0			0x0710
// #define PINCTRL_OFFSET_DRV_CFG_0_SET		0x0714
// #define PINCTRL_OFFSET_DRV_CFG_0_CLR		0x0718
// #define PINCTRL_OFFSET_DRV_CFG_1			0x0720
// #define PINCTRL_OFFSET_DRV_CFG_1_SET		0x0724
// #define PINCTRL_OFFSET_DRV_CFG_1_CLR		0x0728
// #define PINCTRL_OFFSET_DRV_CFG_2			0x0730
// #define PINCTRL_OFFSET_DRV_CFG_2_SET		0x0734
// #define PINCTRL_OFFSET_DRV_CFG_2_CLR		0x0738
// #define PINCTRL_OFFSET_DRV_CFG_3			0x0740
// #define PINCTRL_OFFSET_DRV_CFG_3_SET		0x0744
// #define PINCTRL_OFFSET_DRV_CFG_3_CLR		0x0748
// #define PINCTRL_OFFSET_DRV_CFG_4			0x0750
// #define PINCTRL_OFFSET_DRV_CFG_4_SET		0x0754
// #define PINCTRL_OFFSET_DRV_CFG_4_CLR		0x0758
// #define PINCTRL_OFFSET_DRV_CFG_5			0x0760
// #define PINCTRL_OFFSET_DRV_CFG_5_SET		0x0764
// #define PINCTRL_OFFSET_DRV_CFG_5_CLR		0x0768
// #define PINCTRL_OFFSET_DRV_CFG_6			0x0770
// #define PINCTRL_OFFSET_DRV_CFG_6_SET		0x0774
// #define PINCTRL_OFFSET_DRV_CFG_6_CLR		0x0778
// #define PINCTRL_OFFSET_PULL_EN_0			0x0860
// #define PINCTRL_OFFSET_PULL_EN_0_SET		0x0864
// #define PINCTRL_OFFSET_PULL_EN_0_CLR		0x0868
// #define PINCTRL_OFFSET_PULL_EN_1			0x0870
// #define PINCTRL_OFFSET_PULL_EN_1_SET		0x0874
// #define PINCTRL_OFFSET_PULL_EN_1_CLR		0x0878
// #define PINCTRL_OFFSET_PULL_EN_2			0x0880
// #define PINCTRL_OFFSET_PULL_EN_2_SET		0x0884
// #define PINCTRL_OFFSET_PULL_EN_2_CLR		0x0888
// #define PINCTRL_OFFSET_PULL_EN_3			0x0890
// #define PINCTRL_OFFSET_PULL_EN_3_SET		0x0894
// #define PINCTRL_OFFSET_PULL_EN_3_CLR		0x0898
// #define PINCTRL_OFFSET_PULL_EN_4			0x08a0
// #define PINCTRL_OFFSET_PULL_EN_4_SET		0x08a4
// #define PINCTRL_OFFSET_PULL_EN_4_CLR		0x08a8
// #define PINCTRL_OFFSET_PULL_SEL_0			0x0900
// #define PINCTRL_OFFSET_PULL_SEL_0_SET		0x0904
// #define PINCTRL_OFFSET_PULL_SEL_0_CLR		0x0908
// #define PINCTRL_OFFSET_PULL_SEL_1			0x0910
// #define PINCTRL_OFFSET_PULL_SEL_1_SET		0x0914
// #define PINCTRL_OFFSET_PULL_SEL_1_CLR		0x0918
// #define PINCTRL_OFFSET_PULL_SEL_2			0x0920
// #define PINCTRL_OFFSET_PULL_SEL_2_SET		0x0924
// #define PINCTRL_OFFSET_PULL_SEL_2_CLR		0x0928
// #define PINCTRL_OFFSET_PULL_SEL_3			0x0930
// #define PINCTRL_OFFSET_PULL_SEL_3_SET		0x0934
// #define PINCTRL_OFFSET_PULL_SEL_3_CLR		0x0938
// #define PINCTRL_OFFSET_PULL_SEL_4			0x0940
// #define PINCTRL_OFFSET_PULL_SEL_4_SET		0x0944
// #define PINCTRL_OFFSET_PULL_SEL_4_CLR		0x0948
// #define PINCTRL_OFFSET_SEC_CFG_1			0x0950
// #define PINCTRL_OFFSET_SEC_CFG_0			0x0960
// #define PINCTRL_OFFSET_MISC_DUMMY			0x09d0
// #define PINCTRL_OFFSET_BIAS_CFG             0x09f0

typedef struct {
	uint16_t max_pin;
	uint16_t max_func;
} pinctrl_mtk_config_t;

static const pinctrl_mtk_config_t pinctrl_mtk_config = {
	.max_pin = 177,
	.max_func = 8,
};

static const uint32_t pin_to_mode_offset_map[] = {
	/*   0 -   7 */ PINCTRL_OFFSET_MODE_0,
	/*   8 -  15 */ PINCTRL_OFFSET_MODE_1,
	/*  16 -  23 */ PINCTRL_OFFSET_MODE_2,
	/*  24 -  31 */ PINCTRL_OFFSET_MODE_3,
	/*  32 -  39 */ PINCTRL_OFFSET_MODE_4,
	/*  40 -  47 */ PINCTRL_OFFSET_MODE_5,
	/*  48 -  55 */ PINCTRL_OFFSET_MODE_6,
	/*  56 -  63 */ PINCTRL_OFFSET_MODE_7,
	/*  64 -  71 */ PINCTRL_OFFSET_MODE_8,
	/*  72 -  79 */ PINCTRL_OFFSET_MODE_9,
	/*  80 -  87 */ PINCTRL_OFFSET_MODE_10,
	/*  88 -  95 */ PINCTRL_OFFSET_MODE_11,
	/*  96 - 103 */ PINCTRL_OFFSET_MODE_12,
	/* 104 - 111 */ PINCTRL_OFFSET_MODE_13,
	/* 112 - 119 */ PINCTRL_OFFSET_MODE_14,
	/* 120 - 127 */ PINCTRL_OFFSET_MODE_15,
	/* 128 - 135 */ PINCTRL_OFFSET_MODE_16,
	/* 136 - 143 */ PINCTRL_OFFSET_MODE_17,
	/* 144 - 151 */ PINCTRL_OFFSET_MODE_18,
	/* 152 - 159 */ PINCTRL_OFFSET_MODE_19,
	/* 160 - 167 */ PINCTRL_OFFSET_MODE_20,
	/* 168 - 175 */ PINCTRL_OFFSET_MODE_21,
	/* 176 - 176 */ PINCTRL_OFFSET_MODE_22,
	/* 177 - 183 - Reserved */
};

static uint32_t to_mode_offset(uint16_t pin);
static uint32_t to_mode_shift(uint16_t pin);
static uint32_t to_mode_mask(uint16_t pin);

static uint32_t to_mode_offset(uint16_t pin)
{
	/* Each 32 bit MODE register controls 8 pins. */
	return pin_to_mode_offset_map[pin / 8];
}

static uint32_t to_mode_shift(uint16_t pin)
{
	return ((pin % 8) * 4);
}

static uint32_t to_mode_mask(uint16_t pin)
{
	return (0b111 << to_mode_shift(pin));
}

static int pinctrl_set_func(uint16_t pin, uint16_t func)
{
	uint32_t val;
	uint32_t offset;

	if ((pin >= pinctrl_mtk_config.max_pin) || (func >= pinctrl_mtk_config.max_func))
		return -EINVAL;

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
    ARG_UNUSED(reg);

	int ret;

    for (int idx = 0; idx < pin_cnt; idx++) {
		ret = pinctrl_set_func(pins[idx].pin, pins[idx].func);
		if (ret != 0)
			return (ret);
	}

	return 0;
}

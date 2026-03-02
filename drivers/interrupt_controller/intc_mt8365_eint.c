/*
 * Copyright (c) 2025 MediaTek
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT mediatek_mt8365_eint

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/irq.h>

#include "intc_mtk_eint_common.h"

#define EINT_OFFSET_STA_0              0x0000
#define EINT_OFFSET_STA_1              0x0004
#define EINT_OFFSET_STA_2              0x0008
#define EINT_OFFSET_STA_3              0x000c
#define EINT_OFFSET_STA_4              0x0010
#define EINT_OFFSET_ACK_0              0x0040
#define EINT_OFFSET_ACK_1              0x0044
#define EINT_OFFSET_ACK_2              0x0048
#define EINT_OFFSET_ACK_3              0x004c
#define EINT_OFFSET_ACK_4              0x0050
#define EINT_OFFSET_MASK_0             0x0080
#define EINT_OFFSET_MASK_1             0x0084
#define EINT_OFFSET_MASK_2             0x0088
#define EINT_OFFSET_MASK_3             0x008c
#define EINT_OFFSET_MASK_4             0x0090
#define EINT_OFFSET_MASK_SET_0         0x00c0
#define EINT_OFFSET_MASK_SET_1         0x00c4
#define EINT_OFFSET_MASK_SET_2         0x00c8
#define EINT_OFFSET_MASK_SET_3         0x00cc
#define EINT_OFFSET_MASK_SET_4         0x00d0
#define EINT_OFFSET_MASK_CLR_0         0x0100
#define EINT_OFFSET_MASK_CLR_1         0x0104
#define EINT_OFFSET_MASK_CLR_2         0x0108
#define EINT_OFFSET_MASK_CLR_3         0x010c
#define EINT_OFFSET_MASK_CLR_4         0x0110
#define EINT_OFFSET_SENS_0             0x0140
#define EINT_OFFSET_SENS_1             0x0144
#define EINT_OFFSET_SENS_2             0x0148
#define EINT_OFFSET_SENS_3             0x014c
#define EINT_OFFSET_SENS_4             0x0150
#define EINT_OFFSET_SENS_SET_0         0x0180
#define EINT_OFFSET_SENS_SET_1         0x0184
#define EINT_OFFSET_SENS_SET_2         0x0188
#define EINT_OFFSET_SENS_SET_3         0x018c
#define EINT_OFFSET_SENS_SET_4         0x0190
#define EINT_OFFSET_SENS_CLR_0         0x01c0
#define EINT_OFFSET_SENS_CLR_1         0x01c4
#define EINT_OFFSET_SENS_CLR_2         0x01c8
#define EINT_OFFSET_SENS_CLR_3         0x01cc
#define EINT_OFFSET_SENS_CLR_4         0x01d0
#define EINT_OFFSET_SOFT_0             0x0200
#define EINT_OFFSET_SOFT_1             0x0204
#define EINT_OFFSET_SOFT_2             0x0208
#define EINT_OFFSET_SOFT_3             0x020c
#define EINT_OFFSET_SOFT_4             0x0210
#define EINT_OFFSET_SOFT_SET_0         0x0240
#define EINT_OFFSET_SOFT_SET_1         0x0244
#define EINT_OFFSET_SOFT_SET_2         0x0248
#define EINT_OFFSET_SOFT_SET_3         0x024c
#define EINT_OFFSET_SOFT_SET_4         0x0250
#define EINT_OFFSET_SOFT_CLR_0         0x0280
#define EINT_OFFSET_SOFT_CLR_1         0x0284
#define EINT_OFFSET_SOFT_CLR_2         0x0288
#define EINT_OFFSET_SOFT_CLR_3         0x028c
#define EINT_OFFSET_SOFT_CLR_4         0x0290
#define EINT_OFFSET_POL_0              0x0300
#define EINT_OFFSET_POL_1              0x0304
#define EINT_OFFSET_POL_2              0x0308
#define EINT_OFFSET_POL_3              0x030c
#define EINT_OFFSET_POL_4              0x0310
#define EINT_OFFSET_POL_SET_0          0x0340
#define EINT_OFFSET_POL_SET_1          0x0344
#define EINT_OFFSET_POL_SET_2          0x0348
#define EINT_OFFSET_POL_SET_3          0x034c
#define EINT_OFFSET_POL_SET_4          0x0350
#define EINT_OFFSET_POL_CLR_0          0x0380
#define EINT_OFFSET_POL_CLR_1          0x0384
#define EINT_OFFSET_POL_CLR_2          0x0388
#define EINT_OFFSET_POL_CLR_3          0x038c
#define EINT_OFFSET_POL_CLR_4          0x0390
#define EINT_OFFSET_D0EN_0             0x0400
#define EINT_OFFSET_D0EN_1             0x0404
#define EINT_OFFSET_D0EN_2             0x0408
#define EINT_OFFSET_D0EN_3             0x040c
#define EINT_OFFSET_D0EN_4             0x0410
#define EINT_OFFSET_DBNC_3_0           0x0500
#define EINT_OFFSET_DBNC_7_4           0x0504
#define EINT_OFFSET_DBNC_B_8           0x0508
#define EINT_OFFSET_DBNC_F_C           0x050c
#define EINT_OFFSET_DBNC_9_3_0         0x0590
#define EINT_OFFSET_DBNC_9_7_4         0x0594
#define EINT_OFFSET_DBNC_SET_3_0       0x0600
#define EINT_OFFSET_DBNC_SET_7_4       0x0604
#define EINT_OFFSET_DBNC_SET_B_8       0x0608
#define EINT_OFFSET_DBNC_SET_F_C       0x060c
#define EINT_OFFSET_DBNC_SET_9_3_0     0x0690
#define EINT_OFFSET_DBNC_SET_9_7_4     0x0694
#define EINT_OFFSET_DBNC_CLR_3_0       0x0700
#define EINT_OFFSET_DBNC_CLR_7_4       0x0704
#define EINT_OFFSET_DBNC_CLR_B_8       0x0708
#define EINT_OFFSET_DBNC_CLR_F_C       0x070c
#define EINT_OFFSET_DBNC_CLR_9_3_0     0x0790
#define EINT_OFFSET_DBNC_CLR_9_7_4     0x0794
#define EINT_OFFSET_DCON               0x0800
#define EINT_OFFSET_DCON_SEL_3_0       0x0840
#define EINT_OFFSET_DCON_SEL_SET_3_0   0x0880
#define EINT_OFFSET_DCON_SEL_CLR_3_0   0x08c0
#define EINT_OFFSET_EEVT               0x0900
#define EINT_OFFSET_RAW_STA_0          0x0a00
#define EINT_OFFSET_RAW_STA_1          0x0a04
#define EINT_OFFSET_RAW_STA_2          0x0a08
#define EINT_OFFSET_RAW_STA_3          0x0a0c
#define EINT_OFFSET_RAW_STA_4          0x0a10
#define EINT_OFFSET_SECURE_EINT_EN     0x0b00
#define EINT_OFFSET_SECURE_DIR_EINT_EN 0x0b10
#define EINT_OFFSET_DCM_ON             0x0b20
#define EINT_OFFSET_SECURE_STATUS      0x0b30
#define EINT_OFFSET_SYNC_EN_0          0x0c00
#define EINT_OFFSET_SYNC_EN_1          0x0c04
#define EINT_OFFSET_SYNC_EN_SET_0      0x0d00
#define EINT_OFFSET_SYNC_EN_SET_1      0x0d04
#define EINT_OFFSET_SYNC_EN_CLR_0      0x0e00
#define EINT_OFFSET_SYNC_EN_CLR_1      0x0e04
#define EINT_OFFSET_FPGA_EMUL_0        0x0f00
#define EINT_OFFSET_FPGA_EMUL_1        0x0f04
#define EINT_OFFSET_FPGA_EMUL_2        0x0f08
#define EINT_OFFSET_FPGA_EMUL_3        0x0f0c
#define EINT_OFFSET_FPGA_EMUL_4        0x0f10

static inline uint32_t line_to_offset(uint8_t line);

static uint32_t line_to_bit_mask(uint8_t line);
static uint32_t offset_sta(uint8_t line);
static uint32_t offset_ack(uint8_t line);
static uint32_t offset_mask(uint8_t line);
static uint32_t offset_mask_set(uint8_t line);
static uint32_t offset_mask_clr(uint8_t line);

static inline uint32_t line_to_offset(uint8_t line)
{
	return ((((uint32_t)line) / 32) * 4);
}

static uint32_t line_to_bit_mask(uint8_t line)
{
	return (1 << (((uint32_t)line) % 32));
}

static uint32_t offset_sta(uint8_t line)
{
	return (EINT_OFFSET_STA_0 + (line_to_offset(line)));
}

static uint32_t offset_ack(uint8_t line)
{
	return (EINT_OFFSET_ACK_0 + (line_to_offset(line)));
}

static uint32_t offset_mask(uint8_t line)
{
	return (EINT_OFFSET_MASK_0 + (line_to_offset(line)));
}

static uint32_t offset_mask_set(uint8_t line)
{
	return (EINT_OFFSET_MASK_SET_0 + (line_to_offset(line)));
}

static uint32_t offset_mask_clr(uint8_t line)
{
	return (EINT_OFFSET_MASK_CLR_0 + (line_to_offset(line)));
}

#define EINT_DECLARE_CFG(n)                                                                        \
	static void irq_config_func_##n(const struct device *dev)                                  \
	{                                                                                          \
		IRQ_CONNECT(DT_INST_IRQN(n), DT_INST_IRQ(n, priority), eint_mtk_isr,               \
			    DEVICE_DT_INST_GET(n), 0);                                             \
		irq_enable(DT_INST_IRQN(n));                                                       \
	}                                                                                          \
                                                                                                   \
	static eint_mtk_data_t eint_mtk_##n##_data;                                                \
                                                                                                   \
	static const eint_mtk_config_t eint_mtk_##n##_config = {                                   \
		DEVICE_MMIO_ROM_INIT(DT_DRV_INST(n)),                                              \
		.num_lines = DT_INST_PROP(n, num_lines),                                           \
		.irq_config = irq_config_func_##n,                                                 \
		.line_to_bit_mask = line_to_bit_mask,                                              \
		.offset_sta = offset_sta,                                                          \
		.offset_ack = offset_ack,                                                          \
		.offset_mask = offset_mask,                                                        \
		.offset_mask_set = offset_mask_set,                                                \
		.offset_mask_clr = offset_mask_clr,                                                \
	};

#define EINT_INIT(n)                                                                               \
	EINT_DECLARE_CFG(n)                                                                        \
	DEVICE_DT_INST_DEFINE(n, &eint_mtk_init, NULL, &eint_mtk_##n##_data,                       \
			      &eint_mtk_##n##_config, PRE_KERNEL_1, CONFIG_INTC_INIT_PRIORITY,     \
			      NULL);

DT_INST_FOREACH_STATUS_OKAY(EINT_INIT)

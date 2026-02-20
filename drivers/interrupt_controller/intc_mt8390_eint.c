/*
 * Copyright (c) 2025 MediaTek
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT mediatek_mt8390_eint

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/interrupt_controller/intc_mt8390_eint.h>
#include <zephyr/irq.h>

#define EINT_OFFSET_STA_0          0x0000
#define EINT_OFFSET_STA_1          0x0004
#define EINT_OFFSET_STA_2          0x0008
#define EINT_OFFSET_STA_3          0x000c
#define EINT_OFFSET_STA_4          0x0010
#define EINT_OFFSET_STA_5          0x0014
#define EINT_OFFSET_STA_6          0x0018
#define EINT_OFFSET_STA_7          0x001c
#define EINT_OFFSET_ACK_0          0x0040
#define EINT_OFFSET_ACK_1          0x0044
#define EINT_OFFSET_ACK_2          0x0048
#define EINT_OFFSET_ACK_3          0x004c
#define EINT_OFFSET_ACK_4          0x0050
#define EINT_OFFSET_ACK_5          0x0054
#define EINT_OFFSET_ACK_6          0x0058
#define EINT_OFFSET_ACK_7          0x005c
#define EINT_OFFSET_MASK_0         0x0080
#define EINT_OFFSET_MASK_1         0x0084
#define EINT_OFFSET_MASK_2         0x0088
#define EINT_OFFSET_MASK_3         0x008c
#define EINT_OFFSET_MASK_4         0x0090
#define EINT_OFFSET_MASK_5         0x0094
#define EINT_OFFSET_MASK_6         0x0098
#define EINT_OFFSET_MASK_7         0x009c
#define EINT_OFFSET_MASK_SET_0     0x00c0
#define EINT_OFFSET_MASK_SET_1     0x00c4
#define EINT_OFFSET_MASK_SET_2     0x00c8
#define EINT_OFFSET_MASK_SET_3     0x00cc
#define EINT_OFFSET_MASK_SET_4     0x00d0
#define EINT_OFFSET_MASK_SET_5     0x00d4
#define EINT_OFFSET_MASK_SET_6     0x00d8
#define EINT_OFFSET_MASK_SET_7     0x00dc
#define EINT_OFFSET_MASK_CLR_0     0x0100
#define EINT_OFFSET_MASK_CLR_1     0x0104
#define EINT_OFFSET_MASK_CLR_2     0x0108
#define EINT_OFFSET_MASK_CLR_3     0x010c
#define EINT_OFFSET_MASK_CLR_4     0x0110
#define EINT_OFFSET_MASK_CLR_5     0x0114
#define EINT_OFFSET_MASK_CLR_6     0x0118
#define EINT_OFFSET_MASK_CLR_7     0x011c
#define EINT_OFFSET_SENS_0         0x0140
#define EINT_OFFSET_SENS_1         0x0144
#define EINT_OFFSET_SENS_2         0x0148
#define EINT_OFFSET_SENS_3         0x014c
#define EINT_OFFSET_SENS_4         0x0150
#define EINT_OFFSET_SENS_5         0x0154
#define EINT_OFFSET_SENS_6         0x0158
#define EINT_OFFSET_SENS_7         0x015c
#define EINT_OFFSET_SENS_SET_0     0x0180
#define EINT_OFFSET_SENS_SET_1     0x0184
#define EINT_OFFSET_SENS_SET_2     0x0188
#define EINT_OFFSET_SENS_SET_3     0x018c
#define EINT_OFFSET_SENS_SET_4     0x0190
#define EINT_OFFSET_SENS_SET_5     0x0194
#define EINT_OFFSET_SENS_SET_6     0x0198
#define EINT_OFFSET_SENS_SET_7     0x019c
#define EINT_OFFSET_SENS_CLR_0     0x01c0
#define EINT_OFFSET_SENS_CLR_1     0x01c4
#define EINT_OFFSET_SENS_CLR_2     0x01c8
#define EINT_OFFSET_SENS_CLR_3     0x01cc
#define EINT_OFFSET_SENS_CLR_4     0x01d0
#define EINT_OFFSET_SENS_CLR_5     0x01d4
#define EINT_OFFSET_SENS_CLR_6     0x01d8
#define EINT_OFFSET_SENS_CLR_7     0x01dc
#define EINT_OFFSET_SOFT_0         0x0200
#define EINT_OFFSET_SOFT_1         0x0204
#define EINT_OFFSET_SOFT_2         0x0208
#define EINT_OFFSET_SOFT_3         0x020c
#define EINT_OFFSET_SOFT_4         0x0210
#define EINT_OFFSET_SOFT_5         0x0214
#define EINT_OFFSET_SOFT_6         0x0218
#define EINT_OFFSET_SOFT_7         0x021c
#define EINT_OFFSET_SOFT_SET_0     0x0240
#define EINT_OFFSET_SOFT_SET_1     0x0244
#define EINT_OFFSET_SOFT_SET_2     0x0248
#define EINT_OFFSET_SOFT_SET_3     0x024c
#define EINT_OFFSET_SOFT_SET_4     0x0250
#define EINT_OFFSET_SOFT_SET_5     0x0254
#define EINT_OFFSET_SOFT_SET_6     0x0258
#define EINT_OFFSET_SOFT_SET_7     0x025c
#define EINT_OFFSET_SOFT_CLR_0     0x0280
#define EINT_OFFSET_SOFT_CLR_1     0x0284
#define EINT_OFFSET_SOFT_CLR_2     0x0288
#define EINT_OFFSET_SOFT_CLR_3     0x028c
#define EINT_OFFSET_SOFT_CLR_4     0x0290
#define EINT_OFFSET_SOFT_CLR_5     0x0294
#define EINT_OFFSET_SOFT_CLR_6     0x0298
#define EINT_OFFSET_SOFT_CLR_7     0x029c
#define EINT_OFFSET_POL_0          0x0300
#define EINT_OFFSET_POL_1          0x0304
#define EINT_OFFSET_POL_2          0x0308
#define EINT_OFFSET_POL_3          0x030c
#define EINT_OFFSET_POL_4          0x0310
#define EINT_OFFSET_POL_5          0x0314
#define EINT_OFFSET_POL_6          0x0318
#define EINT_OFFSET_POL_7          0x031c
#define EINT_OFFSET_POL_SET_0      0x0340
#define EINT_OFFSET_POL_SET_1      0x0344
#define EINT_OFFSET_POL_SET_2      0x0348
#define EINT_OFFSET_POL_SET_3      0x034c
#define EINT_OFFSET_POL_SET_4      0x0350
#define EINT_OFFSET_POL_SET_5      0x0354
#define EINT_OFFSET_POL_SET_6      0x0358
#define EINT_OFFSET_POL_SET_7      0x035c
#define EINT_OFFSET_POL_CLR_0      0x0380
#define EINT_OFFSET_POL_CLR_1      0x0384
#define EINT_OFFSET_POL_CLR_2      0x0388
#define EINT_OFFSET_POL_CLR_3      0x038c
#define EINT_OFFSET_POL_CLR_4      0x0390
#define EINT_OFFSET_POL_CLR_5      0x0394
#define EINT_OFFSET_POL_CLR_6      0x0398
#define EINT_OFFSET_POL_CLR_7      0x039c
#define EINT_OFFSET_D0EN_0         0x0400
#define EINT_OFFSET_D0EN_1         0x0404
#define EINT_OFFSET_D0EN_2         0x0408
#define EINT_OFFSET_D0EN_3         0x040c
#define EINT_OFFSET_D0EN_4         0x0410
#define EINT_OFFSET_D0EN_5         0x0414
#define EINT_OFFSET_D0EN_6         0x0418
#define EINT_OFFSET_DBNC_3_0       0x0500
#define EINT_OFFSET_DBNC_7_4       0x0504
#define EINT_OFFSET_DBNC_B_8       0x0508
#define EINT_OFFSET_DBNC_F_C       0x050c
#define EINT_OFFSET_DBNC_1_3_0     0x0510
#define EINT_OFFSET_DBNC_1_7_4     0x0514
#define EINT_OFFSET_DBNC_1_B_8     0x0518
#define EINT_OFFSET_DBNC_1_F_C     0x051c
#define EINT_OFFSET_DBNC_SET_3_0   0x0600
#define EINT_OFFSET_DBNC_SET_7_4   0x0604
#define EINT_OFFSET_DBNC_SET_B_8   0x0608
#define EINT_OFFSET_DBNC_SET_F_C   0x060c
#define EINT_OFFSET_DBNC_SET_1_3_0 0x0610
#define EINT_OFFSET_DBNC_SET_1_7_4 0x0614
#define EINT_OFFSET_DBNC_SET_1_B_8 0x0618
#define EINT_OFFSET_DBNC_SET_1_F_C 0x061c
#define EINT_OFFSET_DBNC_CLR_3_0   0x0700
#define EINT_OFFSET_DBNC_CLR_7_4   0x0704
#define EINT_OFFSET_DBNC_CLR_B_8   0x0708
#define EINT_OFFSET_DBNC_CLR_F_C   0x070c
#define EINT_OFFSET_DBNC_CLR_1_3_0 0x0710
#define EINT_OFFSET_DBNC_CLR_1_7_4 0x0714
#define EINT_OFFSET_DBNC_CLR_1_B_8 0x0718
#define EINT_OFFSET_DBNC_CLR_1_F_C 0x071c

typedef struct {
	eint_mtk_callback_t callback;
	void *arg;
	uint8_t line;
} eint_callback_t;

typedef struct {
	DEVICE_MMIO_ROM; /* Must be first */
	uint16_t num_lines;
	void (*irq_config_func)(const struct device *dev);
} eint_mtk_config_t;

typedef struct {
	DEVICE_MMIO_RAM; /* Must be first */
	struct k_spinlock lock;
	sys_slist_t eint_callbacks;
} eint_mtk_data_t;

static inline uint32_t line_to_bit_mask(uint8_t line);
static inline uint32_t line_to_offset(uint8_t line);
static inline uint32_t offset_sta(uint8_t line);
static inline uint32_t offset_ack(uint8_t line);
static inline uint32_t offset_mask(uint8_t line);
static inline uint32_t offset_mask_set(uint8_t line);
static inline uint32_t offset_mask_clr(uint8_t line);

static void handle_line_isr(const struct device *dev, uint8_t line, uint32_t bit_mask);
static void eint_mtk_isr(const struct device *dev);
static int eint_mtk_init(const struct device *dev);

static inline uint32_t line_to_bit_mask(uint8_t line)
{
	return (1 << (((uint32_t)line) % 32));
}

static inline uint32_t line_to_offset(uint8_t line)
{
	return ((((uint32_t)line) / 32) * 4);
}

static inline uint32_t offset_sta(uint8_t line)
{
	return (EINT_OFFSET_STA_0 + (line_to_offset(line)));
}

static inline uint32_t offset_ack(uint8_t line)
{
	return (EINT_OFFSET_ACK_0 + (line_to_offset(line)));
}

static inline uint32_t offset_mask(uint8_t line)
{
	return (EINT_OFFSET_MASK_0 + (line_to_offset(line)));
}

static inline uint32_t offset_mask_set(uint8_t line)
{
	return (EINT_OFFSET_MASK_SET_0 + (line_to_offset(line)));
}

static inline uint32_t offset_mask_clr(uint8_t line)
{
	return (EINT_OFFSET_MASK_CLR_0 + (line_to_offset(line)));
}

static void handle_line_isr(const struct device *dev, uint8_t line, uint32_t bit_mask)
{
	eint_mtk_data_t *eint_data = dev->data;
	eint_mtk_callback_t *eint_callback;
	eint_mtk_callback_t *tmp;

	while (bit_mask != 0) {
		if ((bit_mask & 0x00000001) != 0) {
			SYS_SLIST_FOR_EACH_CONTAINER_SAFE(&(eint_data->eint_callbacks),
							eint_callback, tmp, node) {
				if ((line >= eint_callback->first_line) &&
					(line <	 (eint_callback->first_line + eint_callback->num_lines))) {
					eint_callback->cb_handler(eint_callback->cb_dev, line,
								  eint_callback->cb_arg);
				}
			}

			sys_write32(line_to_bit_mask(line), (DEVICE_MMIO_GET(dev) + offset_ack(line)));
		}

		line++;
		bit_mask >>= 1;
	}
}

static void eint_mtk_isr(const struct device *dev)
{
	uint8_t line = 0;
	uint32_t status;
	const eint_mtk_config_t *eint_config = dev->config;

	while (line < eint_config->num_lines) {
		status = sys_read32(DEVICE_MMIO_GET(dev) + offset_sta(line));

		if (status != 0) {
			handle_line_isr(dev, line, status);
		}

		line += 32;
	}
}

static int eint_mtk_init(const struct device *dev)
{
	eint_mtk_data_t *eint_data = dev->data;
	const eint_mtk_config_t *eint_config = dev->config;

	DEVICE_MMIO_MAP(dev, K_MEM_CACHE_NONE);

	sys_slist_init(&(eint_data->eint_callbacks));

	eint_config->irq_config_func(dev);

	return 0;
}

int eint_mtk_init_callback(eint_mtk_callback_t *callback, uint8_t first_line, uint8_t num_lines,
			   eint_mtk_cb_handler_t cb_handler, const struct device *cb_dev,
			   void *cb_arg)
{
	if ((callback == NULL) || (cb_handler == NULL) || (cb_dev == NULL))
		return -EINVAL;

	callback->cb_handler = cb_handler;
	callback->cb_dev = cb_dev;
	callback->cb_arg = cb_arg;
	callback->first_line = first_line;
	callback->num_lines = num_lines;

	return (0);
}

int eint_mtk_add_callback(const struct device *dev, eint_mtk_callback_t *callback)
{
	sys_snode_t *prev_callback;
	eint_mtk_data_t *eint_data = dev->data;
	k_spinlock_key_t key;

	if ((callback == NULL) || (callback->cb_handler == NULL) || (callback->cb_dev == NULL))
		return -EINVAL;

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

	if (line >= eint_config->num_lines)
		return -EINVAL;

	sys_write32(line_to_bit_mask(line), (DEVICE_MMIO_GET(dev) + offset_mask_set(line)));

	return 0;
}

int eint_mtk_disable(const struct device *dev, uint8_t line)
{
	const eint_mtk_config_t *eint_config = dev->config;

	if (line >= eint_config->num_lines)
		return -EINVAL;

	sys_write32(line_to_bit_mask(line), (DEVICE_MMIO_GET(dev) + offset_mask_clr(line)));

	return 0;
}

bool eint_mtk_is_enabled(const struct device *dev, uint8_t line)
{
	const eint_mtk_config_t *eint_config = dev->config;
	uint32_t val;

	if (line >= eint_config->num_lines)
		return false;

	val = sys_read32(DEVICE_MMIO_GET(dev) + offset_mask(line));
	val &= line_to_bit_mask(line);

	return (val != 0);
}

#define EINT_DECLARE_CFG(n, IRQ_FUNC_INIT)                      \
	static const eint_mtk_config_t eint_mtk_##n##_config = {    \
		DEVICE_MMIO_ROM_INIT(DT_DRV_INST(n)),                   \
		.num_lines = DT_INST_PROP(n, num_lines),                \
		IRQ_FUNC_INIT                                           \
	}

#define EINT_IRQ_CONFIG_FUNC(n)                                                 \
	static void irq_config_func_##n(const struct device *dev) {                 \
		IRQ_CONNECT(DT_INST_IRQN(n), DT_INST_IRQ(n, priority), eint_mtk_isr,    \
			DEVICE_DT_INST_GET(n), 0);                                          \
		irq_enable(DT_INST_IRQN(n));                                            \
	}

#define EINT_IRQ_CFG_FUNC_INIT(n)   \
		.irq_config_func = irq_config_func_##n,

#define EINT_INIT_CFG(n)    EINT_DECLARE_CFG(n, EINT_IRQ_CFG_FUNC_INIT(n))

#define EINT_INIT(n)                                                        \
	static eint_mtk_data_t eint_mtk_##n##_data;                             \
	static const eint_mtk_config_t eint_mtk_##n##_config;                   \
	EINT_IRQ_CONFIG_FUNC(n)                                                 \
	DEVICE_DT_INST_DEFINE(n, &eint_mtk_init, NULL, &eint_mtk_##n##_data,    \
		&eint_mtk_##n##_config, PRE_KERNEL_1, CONFIG_INTC_INIT_PRIORITY,    \
		NULL);                                                              \
	EINT_INIT_CFG(n);

DT_INST_FOREACH_STATUS_OKAY(EINT_INIT)

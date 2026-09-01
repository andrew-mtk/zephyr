/*
 * Copyright 2025 MediaTek
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/arch/arm64/arm_mmu.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/util.h>

static const struct arm_mmu_region mmu_regions [] =
{
    MMU_REGION_FLAT_ENTRY ("GIC",
        DT_REG_ADDR_BY_IDX (DT_NODELABEL (gic), 0),
	    DT_REG_SIZE_BY_IDX (DT_NODELABEL (gic), 0),
        (MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS)),

    MMU_REGION_FLAT_ENTRY("GIC",
        DT_REG_ADDR_BY_IDX (DT_NODELABEL (gic), 1),
        DT_REG_SIZE_BY_IDX (DT_NODELABEL (gic), 1),
        (MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS)),

    MMU_REGION_FLAT_ENTRY("PINCTRL",
        DT_REG_ADDR_BY_IDX (DT_NODELABEL (pinctrl), 0),
        DT_REG_SIZE_BY_IDX (DT_NODELABEL (pinctrl), 0),
        (MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS)),

    MMU_REGION_FLAT_ENTRY("EINT",
        DT_REG_ADDR_BY_IDX (DT_NODELABEL (eint), 0),
        DT_REG_SIZE_BY_IDX (DT_NODELABEL (eint), 0),
        (MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS)),

#if DT_NODE_EXISTS(DT_NODELABEL(dma_region))
	/*
	 * DMA buffer region (board DTS). Only zephyr,sram and the .nocache
	 * section are mapped automatically, so a buffer placed in this separate
	 * linker region needs an explicit entry or the first CPU access faults.
	 *
	 * Mapped MT_NORMAL_NC (non-cacheable): the AFE is not cache-coherent
	 * with the CPU, so a cached mapping would let the CPU read stale data
	 * for capture and leave dirty lines unseen by the DMA for playback.
	 */
	MMU_REGION_FLAT_ENTRY("DMA_REGION",
			      DT_REG_ADDR(DT_NODELABEL(dma_region)),
			      DT_REG_SIZE(DT_NODELABEL(dma_region)),
			      (MT_NORMAL_NC | MT_P_RW_U_NA | MT_NS)),
#endif
};

const struct arm_mmu_config mmu_config = {
	.num_regions = ARRAY_SIZE(mmu_regions),
	.mmu_regions = mmu_regions,
};

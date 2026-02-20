# Copyright 2025 MediaTek
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_SOC_MT8390)
  zephyr_include_directories(.)
  zephyr_include_directories(a55)

  zephyr_sources(
    a55/soc.c
  )

  zephyr_sources_ifdef(CONFIG_ARM_MMU a55/mmu_regions.c)

  set(SOC_LINKER_SCRIPT ${ZEPHYR_BASE}/include/zephyr/arch/arm64/scripts/linker.ld CACHE INTERNAL "")
endif()

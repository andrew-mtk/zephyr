/*
 * Copyright (c) 2026 MediaTek Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * MT8188 AFE (Audio Front End) driver — public API and internal structs.
 */

#ifndef DRIVERS_AUDIO_MEDIATEK_MT8188_AFE_H
#define DRIVERS_AUDIO_MEDIATEK_MT8188_AFE_H

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/device_mmio.h>

/* -------------------------------------------------------------------------
 * eTDM format (FMT field in ETDM_IN/OUT_CON0 bits [10:8])
 * -------------------------------------------------------------------------
 */
enum mt8188_etdm_fmt {
	MT8188_ETDM_FMT_I2S   = 0,  /* I2S — LRCK idles low */
	MT8188_ETDM_FMT_RJ    = 1,  /* Right Justified */
	MT8188_ETDM_FMT_LJ    = 2,  /* Left Justified */
	MT8188_ETDM_FMT_EIAJ  = 3,  /* EIAJ CP-1201 */
	MT8188_ETDM_FMT_DSPA  = 4,  /* DSP-A */
	MT8188_ETDM_FMT_DSPB  = 5,  /* DSP-B */
};

/* -------------------------------------------------------------------------
 * Data mode: single I2S pin (one N-ch TDM stream) vs multi-pin (N×2-ch)
 * -------------------------------------------------------------------------
 */
enum mt8188_etdm_data_mode {
	MT8188_ETDM_DATA_ONE_PIN   = 0,
	MT8188_ETDM_DATA_MULTI_PIN = 1,
};

/* -------------------------------------------------------------------------
 * Public configure struct — passed by callers to configure()
 * -------------------------------------------------------------------------
 */
/* MCLK direction (matches Linux SND_SOC_CLOCK_IN / SND_SOC_CLOCK_OUT) */
#define MT8188_MCLK_DIR_IN   0   /* MCLK is an input (external master) */
#define MT8188_MCLK_DIR_OUT  1   /* SoC drives MCLK out to the codec */

struct mt8188_afe_cfg {
	uint32_t             rate;         /* sample rate, e.g. 48000 */
	uint32_t             channels;     /* number of channels */
	uint8_t              word_size;    /* bits per slot: 16 or 32 */
	enum mt8188_etdm_fmt fmt;          /* eTDM format (I2S, DSP-A/B, ...) */
	enum mt8188_etdm_data_mode data_mode; /* single-pin vs multi-pin */
	bool                 slave_mode;   /* true: this stream's eTDM port takes
					    * BCK/LRCK from its own pins (an
					    * external master drives them).
					    * Capture only — eTDM_OUT1 is
					    * master-only. Cowork slave ports
					    * are set internally by the driver.
					    */
	uint32_t             slots;        /* TDM slot count (0 = use channels) */
	uint32_t             lrck_width;   /* LRCK width in BCKs (0 = auto) */
	uint32_t             mclk_freq;    /* MCLK output freq in Hz (0 = no MCLK).
					    * Calibrated against the APLL at
					    * configure() time.
					    */
	int                  mclk_dir;     /* MT8188_MCLK_DIR_IN / _OUT */
	uint32_t             period_frames; /* frames per IRQ period (e.g. 480 for
					     * 10 ms at 48 kHz). Stored at configure()
					     * time and written to irq_cnt_reg at
					     * start(). Linux: runtime->period_size.
					     */
};

/* -------------------------------------------------------------------------
 * Memory interface IDs (DL = downlink/playback, UL = uplink/capture)
 * -------------------------------------------------------------------------
 */
enum mt8188_memif_id {
	MT8188_DL2 = 0,
	MT8188_DL3,
	MT8188_DL6,
	MT8188_DL7,
	MT8188_DL8,
	MT8188_DL10,
	MT8188_DL11,
	MT8188_UL1,
	MT8188_UL2,
	MT8188_UL3,
	MT8188_UL4,
	MT8188_UL5,
	MT8188_UL6,
	MT8188_UL8,
	MT8188_UL9,
	MT8188_UL10,
	MT8188_MEMIF_NR,
};

/* -------------------------------------------------------------------------
 * Route source and destination IDs for the public route() API
 * -------------------------------------------------------------------------
 */
enum mt8188_route_src {
	MT8188_ROUTE_SRC_ETDM_IN1 = 0,
	MT8188_ROUTE_SRC_ETDM_IN2,
	/* Merged eTDM_IN1 + eTDM_IN2 (32ch) via AFE_CONN; CM0 in bypass */
	MT8188_ROUTE_SRC_ETDM_IN1_IN2,
	/* Playback sources */
	MT8188_ROUTE_SRC_DL8,   /* I046..I061 via the DL8_DL11 mux (16ch) */
	MT8188_ROUTE_SRC_DL11,  /* I022..I037 direct (16ch), or +mux for 32ch */
	MT8188_ROUTE_SRC_NR,
};

enum mt8188_route_dst {
	MT8188_ROUTE_DST_UL3 = 0,
	MT8188_ROUTE_DST_UL8,
	MT8188_ROUTE_DST_UL9,
	/* Playback destinations */
	MT8188_ROUTE_DST_ETDM_OUT1,      /* O072..O087 */
	MT8188_ROUTE_DST_ETDM_OUT2,      /* O048..O063 */
	/* DL11 32ch: O072..O087 (ch0-15) + O048..O063 (ch16-31) */
	MT8188_ROUTE_DST_ETDM_OUT1_OUT2,
	MT8188_ROUTE_DST_NR,
};

/* -------------------------------------------------------------------------
 * eTDM port IDs
 * -------------------------------------------------------------------------
 */
enum mt8188_etdm_id {
	MT8188_ETDM_OUT1 = 0,
	MT8188_ETDM_OUT2,
	MT8188_ETDM_IN1,
	MT8188_ETDM_IN2,
	MT8188_ETDM_NR,
};

/* -------------------------------------------------------------------------
 * Public driver API (custom — not Zephyr DAI subsystem)
 * -------------------------------------------------------------------------
 */
struct mt8188_afe_driver_api {
	/**
	 * @brief Configure a memif (sample rate, channels, word size, format).
	 *
	 * Must be called before start(). For DL memifs the format configures
	 * the associated eTDM OUT port; for UL memifs it configures eTDM IN.
	 */
	int (*configure)(const struct device *dev, enum mt8188_memif_id memif_id,
			 const struct mt8188_afe_cfg *cfg);

	/**
	 * @brief Set audio routing connection (src → dst).
	 *
	 * Writes the appropriate AFE_CONN* register bits. Must be called
	 * before start().
	 */
	int (*route)(const struct device *dev,
		     enum mt8188_route_src src, enum mt8188_route_dst dst);

	/** @brief Start DMA on the given memif. */
	int (*start)(const struct device *dev, enum mt8188_memif_id memif_id);

	/** @brief Stop DMA on the given memif. */
	int (*stop)(const struct device *dev, enum mt8188_memif_id memif_id);

	/**
	 * @brief Set the DMA ring buffer.
	 *
	 * @param base     Physical base address of the buffer. Callers must
	 *                 convert virtual addresses to physical using
	 *                 k_mem_phys_addr().
	 * @param buf_size Buffer size in bytes. The driver programs reg_end with
	 *                 base + buf_size - 1 (address of the last byte), matching
	 *                 Linux mtk_memif_set_addr().
	 */
	int (*set_buf)(const struct device *dev, enum mt8188_memif_id memif_id,
		       uint32_t base, uint32_t buf_size);

	/** @brief Read the current DMA write/read pointer (physical address). */
	uint32_t (*get_cur)(const struct device *dev, enum mt8188_memif_id memif_id);

	/**
	 * @brief Register a period-elapsed callback for a memif.
	 *
	 * The callback is invoked from the AFE ISR each time the DMA pointer
	 * crosses a period boundary (i.e., the hardware IRQ fires).
	 * Pass NULL to unregister.
	 *
	 * @param cb      Callback function, or NULL.
	 * @param cb_data Opaque pointer passed to the callback.
	 */
	int (*set_period_cb)(const struct device *dev, enum mt8188_memif_id memif_id,
			     void (*cb)(void *data), void *cb_data);
};

/* -------------------------------------------------------------------------
 * Cowork sync-source IDs (match Linux ETDM_SYNC_* enum values)
 * -------------------------------------------------------------------------
 */
enum mt8188_etdm_cowork_id {
	MT8188_COWORK_ETDM_NONE    = 0,
	MT8188_COWORK_ETDM_IN1_M   = 2,
	MT8188_COWORK_ETDM_IN1_S   = 3,
	MT8188_COWORK_ETDM_IN2_M   = 4,
	MT8188_COWORK_ETDM_IN2_S   = 5,
	MT8188_COWORK_ETDM_OUT1_M  = 10,
	MT8188_COWORK_ETDM_OUT1_S  = 11,
	MT8188_COWORK_ETDM_OUT2_M  = 12,
	MT8188_COWORK_ETDM_OUT2_S  = 13,
};

/* Sentinel for "this port has no cowork master", stored in
 * struct mt8188_etdm_config::cowork_source_id.
 *
 * It must not collide with any enum mt8188_etdm_id value, so it cannot be 0:
 * MT8188_ETDM_OUT1 is 0, so using MT8188_COWORK_ETDM_NONE (also 0) here made
 * a port slaved to eTDM_OUT1 indistinguishable from an unslaved one —
 * mt8188_etdm_update_sync_info() dropped it and the slave port was then never
 * configured at all.
 */
#define MT8188_COWORK_SOURCE_NONE   (-1)

/* Cowork sync-source selectors (match Linux ETDM_SYNC_FROM_* values) */
#define MT8188_ETDM_SYNC_NONE       0
#define MT8188_ETDM_SYNC_FROM_IN1   2
#define MT8188_ETDM_SYNC_FROM_IN2   4
#define MT8188_ETDM_SYNC_FROM_OUT1  10
#define MT8188_ETDM_SYNC_FROM_OUT2  12

/* FS timing tokens for AFIFO and OUT relatch (from Linux mt8188-afe-common.h) */
#define MT8188_ETDM_OUT1_1X_EN  9
#define MT8188_ETDM_OUT2_1X_EN  10
#define MT8188_ETDM_IN1_1X_EN   12
#define MT8188_ETDM_IN2_1X_EN   13
/* Nx_EN tokens — memif FS source for memifs wired directly to an eTDM IN */
#define MT8188_ETDM_IN1_NX_EN   25
#define MT8188_ETDM_IN2_NX_EN   26

#define MT8188_ETDM_MAX_CHANNELS 16
#define MT8188_ETDM_NORMAL_MAX_BCK_RATE 24576000U

/* -------------------------------------------------------------------------
 * Internal per-eTDM-port configuration (stored in struct mt8188_afe)
 * Mirrors Linux struct mtk_dai_etdm_priv.
 * -------------------------------------------------------------------------
 */
struct mt8188_etdm_config {
	enum mt8188_etdm_data_mode data_mode;
	bool     slave_mode;
	bool     lrck_inv;
	bool     bck_inv;
	uint32_t rate;
	enum mt8188_etdm_fmt fmt;
	uint32_t slots;        /* TDM slot override (0 = use cfg->channels) */
	uint32_t lrck_width;   /* LRCK width override in BCKs (0 = auto) */
	uint32_t mclk_freq;    /* MCLK output frequency (0 = disabled) */
	uint32_t mclk_apll;    /* APLL source for MCLK: 1=APLL1, 2=APLL2 */
	int      mclk_dir;     /* SND_SOC_CLOCK_IN / SND_SOC_CLOCK_OUT equiv */
	int      cowork_source_id;  /* enum mt8188_etdm_id of the cowork master,
				     * or MT8188_COWORK_SOURCE_NONE
				     */
	uint32_t cowork_slv_count;
	int      cowork_slv_id[MT8188_ETDM_NR - 1];
	bool     in_disable_ch[MT8188_ETDM_MAX_CHANNELS];
	bool     configured;
};

/* Per-memif runtime state — populated by configure(), consumed by start() */
struct mt8188_memif_rt {
	uint32_t period_frames; /* frames per IRQ period from mt8188_afe_cfg */
	uint32_t rate;          /* sample rate, for uplink start delay */
	uint32_t channels;      /* channel count, for uplink start delay */
	uint8_t  word_size;     /* bits per sample, for uplink start delay */
};

/* Per-memif period callback state */
struct mt8188_memif_cb {
	void (*cb)(void *data);
	void *data;
};

/* -------------------------------------------------------------------------
 * Internal driver state (struct mt8188_afe)
 * -------------------------------------------------------------------------
 */
struct mt8188_afe {
	DEVICE_MMIO_RAM; /* Must be first — holds mapped virtual address */
	uintptr_t base;           /* Cached copy of DEVICE_MMIO_GET(dev) set in init */
	uintptr_t adsp_audio26m;  /* Mapped virtual address of ADSP audio26m block */
	/* Clock controller device handles — one per tier */
	const struct device *clk_apmixed;    /* &apmixedsys */
	const struct device *clk_topckgen;   /* &topckgen   */
	const struct device *clk_infra_a0;   /* &clk_infra_a0 */
	struct k_spinlock lock;
	struct mt8188_etdm_config etdm[MT8188_ETDM_NR];
	/* Per-memif runtime state (populated by configure) */
	struct mt8188_memif_rt memif_rt[MT8188_MEMIF_NR];
	/* Per-memif period-elapsed callbacks (set via set_period_cb API) */
	struct mt8188_memif_cb period_cb[MT8188_MEMIF_NR];
};

/* -------------------------------------------------------------------------
 * Internal clock control functions (mt8188-afe-clk.c)
 * -------------------------------------------------------------------------
 */
int mt8188_afe_enable_reg_rw_clk(struct mt8188_afe *afe);
int mt8188_afe_disable_reg_rw_clk(struct mt8188_afe *afe);
void mt8188_afe_enable_main_clock(struct mt8188_afe *afe);
void mt8188_afe_disable_main_clock(struct mt8188_afe *afe);
int mt8188_apll1_enable(struct mt8188_afe *afe);
int mt8188_apll1_disable(struct mt8188_afe *afe);
int mt8188_apll2_enable(struct mt8188_afe *afe);
int mt8188_apll2_disable(struct mt8188_afe *afe);
int afe_enable_base_clocks(struct mt8188_afe *afe);
int afe_disable_base_clocks(struct mt8188_afe *afe);
/* Rate-selected APLL timing domain (48k family → APLL1/a1sys,
 * 44.1k family → APLL2/a2sys). One domain per stream — enable once even when
 * the stream drives two eTDM ports.
 */
int mt8188_afe_enable_apll_domain(struct mt8188_afe *afe, uint32_t rate);
int mt8188_afe_disable_apll_domain(struct mt8188_afe *afe, uint32_t rate);

/* Per-eTDM-port clocks (MCLK divider + MCLK mux + AUDSYS gate). Call once per
 * port a stream uses: one for a normal path, twice for the cowork paths
 * (UL9 = IN1+IN2, DL11 32ch = OUT1+OUT2).
 */
int mt8188_afe_enable_etdm_clocks(struct mt8188_afe *afe, enum mt8188_etdm_id id);
int mt8188_afe_disable_etdm_clocks(struct mt8188_afe *afe, enum mt8188_etdm_id id);

/* -------------------------------------------------------------------------
 * eTDM DAI functions (mt8188-dai-etdm.c)
 * -------------------------------------------------------------------------
 */

/* Primary configure: called from mt8188_afe_api_configure() */
int mt8188_etdm_configure(struct mt8188_afe *afe, enum mt8188_etdm_id id,
			  const struct mt8188_afe_cfg *cfg);

/* Optional pre-configure overrides (call before configure()) */
int mt8188_etdm_set_fmt(struct mt8188_afe *afe, enum mt8188_etdm_id id,
			enum mt8188_etdm_fmt fmt,
			bool lrck_inv, bool bck_inv, bool slave_mode);
int mt8188_etdm_set_tdm_slot(struct mt8188_afe *afe, enum mt8188_etdm_id id,
			     uint32_t slots, uint32_t lrck_width);
/* set_sysclk stores freq+dir and calls cal_mclk to validate and select APLL */
int mt8188_etdm_set_sysclk(struct mt8188_afe *afe, enum mt8188_etdm_id id,
			   uint32_t mclk_freq, int mclk_dir);
/* Validate mclk_freq against APLL rate and store (called by set_sysclk) */
int mt8188_etdm_cal_mclk(struct mt8188_afe *afe, enum mt8188_etdm_id id,
			  uint32_t freq);
/* Program divider, enable gate, set tuner — call from start() */
int mt8188_etdm_enable_mclk(struct mt8188_afe *afe, enum mt8188_etdm_id id);
/* Disable gate and tuner — call from stop() */
int mt8188_etdm_disable_mclk(struct mt8188_afe *afe, enum mt8188_etdm_id id);
int mt8188_etdm_set_data_mode(struct mt8188_afe *afe, enum mt8188_etdm_id id,
			      enum mt8188_etdm_data_mode mode);
int mt8188_etdm_set_cowork_source(struct mt8188_afe *afe, enum mt8188_etdm_id id,
				  int cowork_source_id);
int mt8188_etdm_set_in_disable_ch(struct mt8188_afe *afe, enum mt8188_etdm_id id,
				  const uint8_t *disabled_chs, uint32_t count);

int mt8188_etdm_start(struct mt8188_afe *afe, enum mt8188_etdm_id id);
int mt8188_etdm_stop(struct mt8188_afe *afe, enum mt8188_etdm_id id);
int mt8188_afe_route(struct mt8188_afe *afe,
		     enum mt8188_route_src src, enum mt8188_route_dst dst);

/* Called once after all set_cowork_source() calls, before configure() */
void mt8188_etdm_update_sync_info(struct mt8188_afe *afe);

#endif /* DRIVERS_AUDIO_MEDIATEK_MT8188_AFE_H */

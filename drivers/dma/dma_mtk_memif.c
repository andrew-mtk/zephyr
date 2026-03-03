
#include <errno.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/drivers/dma.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/device_runtime.h>

#define DT_DRV_COMPAT mediatek_memif
#define MEMIF_FS2REG_CNT (DT_PROP_LEN(DT_DRV_INST(0), fs2reg) / 2)
#define MEMIF_CHANNELS_COUNT   DT_CHILD_NUM_STATUS_OKAY(DT_DRV_INST(0))

LOG_MODULE_REGISTER(mtk_memif);

enum comp_state_type {
	COMP_STATE_NOT_EXIST = 0,	/**< Component does not exist */
	COMP_STATE_INIT,		/**< Component being initialised */
	COMP_STATE_READY,		/**< Component inactive, but ready */
	COMP_STATE_SUSPEND,		/**< Component suspended */
	COMP_STATE_PREPARE,		/**< Component prepared */
	COMP_STATE_PAUSED,		/**< Component paused */
	COMP_STATE_ACTIVE,		/**< Component active */
	COMP_STATE_PRE_ACTIVE	/**< Component after early initialisation */
};
enum comp_frame_format {
	DAI_FRAME_S16_LE = 0,
	DAI_FRAME_S24_4LE,
	DAI_FRAME_S32_LE,
};

	struct memif_channel_conf {
	int id;
	const char *name;
	bool downlink;
	int reg_ofs_base;
	int reg_ofs_cur;
	int reg_ofs_end;
	int reg_ofs_base_msb;
	int reg_ofs_cur_msb;
	int reg_ofs_end_msb;
	int fs_reg;
	int fs_shift;
	int fs_maskbit;
	int mono_reg;
	int mono_shift;
	int mono_invert;
	int quad_ch_reg;
	int quad_ch_mask;
	int quad_ch_shift;
	int int_odd_flag_reg;
	int int_odd_flag_shift;
	int enable_reg;
	int enable_shift;
	int hd_reg;
	int hd_shift;
	int hd_align_reg;
	int hd_align_mshift;
	int msb_reg;
	int msb_shift;
	int msb2_reg;
	int msb2_shift;
	int agent_disable_reg;
	int agent_disable_shift;
	int ch_num_reg;
	int ch_num_shift;
	int ch_num_maskbit;
	/* playback memif only */
	int pbuf_reg;
	int pbuf_mask;
	int pbuf_shift;
	int minlen_reg;
	int minlen_mask;
	int minlen_shift;
};

struct fs2reg_map {
	int fs;
	int reg_val;
};
struct memif_conf {
	int fs2reg_cnt;		/**< Number of fs2reg elements */
	union {
		struct fs2reg_map fs2reg[MEMIF_FS2REG_CNT];
		int fs2reg_array[MEMIF_FS2REG_CNT * 2];
	};
	int ch_cnt;
	const struct memif_channel_conf ch_conf[MEMIF_CHANNELS_COUNT];
};
struct memif_channel_data {
	int memif_id;
	// int dai_id;
	// int irq_id;
//	struct mtk_base_afe *afe; //TODO, add afe pointer if needed

	uint32_t dma_base;
	uint32_t dma_size;
	uint32_t rptr;
	uint32_t wptr;
	uint32_t free;
	uint32_t pending_length;

	uint32_t period_size;

	unsigned int channels;
	unsigned int fs;
	unsigned int format;

	enum comp_state_type status;
};

struct memif_data {
	struct dma_context ctx;
	struct memif_channel_data ch_data[MEMIF_CHANNELS_COUNT];
};

struct memif_dai_config {
	// int dai_id;
	// int memif_id;
	int fs;
	int channels;
	// int format;	
	int src_width;
};

#define AFE_REG_BASE DT_REG_ADDR(DT_DRV_INST(0))
#define AFE_REG_SIZE DT_REG_SIZE(DT_DRV_INST(0))
#define AFE ((volatile uint32_t*)(AFE_REG_BASE))

static struct memif_channel_data* dma_chan_get_data(const struct device *dev,
					   uint32_t chan_id)
{
	const struct memif_conf *conf = dev->config;
	struct memif_data *data = dev->data;

	/* check for index out of bounds */
	// if (chan_id >= data->ctx.dma_channels) {
	if (chan_id >= conf->ch_cnt) {
		LOG_ERR("Invalid channel id: %d", chan_id);
		return NULL;
	}

	return &data->ch_data[chan_id];
}
static const struct memif_channel_conf* dma_chan_get_conf(const struct device *dev,
					   uint32_t chan_id)
{
	const struct memif_conf *conf = dev->config;
	struct memif_data *data = dev->data;

	data = dev->data;
	conf = dev->config;

	/* check for index out of bounds */
//	if (chan_id >= data->ctx.dma_channels) {
	if (chan_id >= conf->ch_cnt) {
		LOG_ERR("Invalid channel id: %d", chan_id);
		return NULL;
	}

	return &conf->ch_conf[chan_id];
}
static inline void afe_reg_read(uint32_t reg, uint32_t *value)
{
	if (reg >= AFE_REG_SIZE) {
		LOG_ERR("Invalid register offset: 0x%x", reg);
		return;
	}

	*value = AFE[reg/sizeof(AFE[0])];
	LOG_DBG("r_reg:0x%x, value:0x%x\n", reg, *value);
}
static inline void afe_reg_write(uint32_t reg, uint32_t value)
{
	if (reg >= AFE_REG_SIZE) {
		LOG_ERR("Invalid register offset: 0x%x", reg);
		return;
	}

	AFE[reg/sizeof(AFE[0])] = value;
	LOG_DBG("w_reg:0x%x, value:0x%x\n", reg, value);
}
static inline void afe_reg_update_bits(uint32_t reg, uint32_t mask, uint32_t value)
{
	if (reg >= AFE_REG_SIZE) {
		LOG_ERR("Invalid register offset: 0x%x", reg);
		return;
	}

	AFE[reg/sizeof(AFE[0])] &= ~mask;
	AFE[reg/sizeof(AFE[0])] |= value;
	LOG_DBG("u_reg:0x%x, value:0x%x\n", reg, value);
}
static int afe_memif_set_channels(const struct device *dev, uint32_t chan_id, unsigned int channels)
{
	const struct memif_channel_conf *ch_conf = dma_chan_get_conf(dev, chan_id);
	
	unsigned int mono;

	if (ch_conf->ch_num_reg >= 0) {
		afe_reg_update_bits(ch_conf->ch_num_reg,
				    ch_conf->ch_num_maskbit << ch_conf->ch_num_shift,
				    channels << ch_conf->ch_num_shift);
	}

	if (ch_conf->quad_ch_mask) {
		unsigned int quad_ch = (channels == 4);

		afe_reg_update_bits(ch_conf->quad_ch_reg,
				    ch_conf->quad_ch_mask << ch_conf->quad_ch_shift,
				    quad_ch << ch_conf->quad_ch_shift);
	}

	mono = (bool)ch_conf->mono_invert ^ (channels == 1);

	if (ch_conf->int_odd_flag_reg > 0)
		afe_reg_update_bits(ch_conf->int_odd_flag_reg,
				    1 << ch_conf->int_odd_flag_shift,
				    mono << ch_conf->int_odd_flag_shift);

	if (ch_conf->mono_reg > 0 && ch_conf->mono_shift >= 0)
		afe_reg_update_bits(ch_conf->mono_reg,
				    1 << ch_conf->mono_shift,
				    mono << ch_conf->mono_shift);
	return 0;
}
static int afe_memif_fs2reg(const struct device *dev, unsigned int rate)
{
	const struct memif_conf *conf = dev->config;
	int i;

	for (i = 0; i < conf->fs2reg_cnt; i++) {
		if (conf->fs2reg[i].fs == rate)
			return conf->fs2reg[i].reg_val;
	}

	return -1;
}

static int afe_memif_set_rate(const struct device *dev, uint32_t chan_id, unsigned int rate)
{
	const struct memif_channel_conf *ch_conf = dma_chan_get_conf(dev, chan_id);
	int fs = 0;

	fs = afe_memif_fs2reg(dev, rate);
	if (fs < 0) {
		LOG_ERR("invalid fs:%d\n", fs);
		return -EINVAL;
	}

	if (ch_conf->fs_reg >= 0)
		afe_reg_update_bits(ch_conf->fs_reg,
				    ch_conf->fs_maskbit << ch_conf->fs_shift,
				    fs << ch_conf->fs_shift);

	return 0;
}
static int afe_memif_set_format(const struct device *dev, uint32_t chan_id, unsigned int format)
{
	const struct memif_channel_conf *ch_conf = dma_chan_get_conf(dev, chan_id);
	int hd_audio;
	int memif_32bit_supported = 0; // TODO afe->memif_32bit_supported; - read from DTS

	/* set hd mode */
	switch (format) {
	case DAI_FRAME_S16_LE:
		hd_audio = 0;
		break;
	case DAI_FRAME_S32_LE:
	case DAI_FRAME_S24_4LE:
		if (memif_32bit_supported)
			hd_audio = 2;
		else
			hd_audio = 1;
		break;
	default:
		LOG_ERR("not support format:%u\n", format);
		return -EINVAL;
	}

	if (ch_conf->hd_reg >= 0)
		afe_reg_update_bits(ch_conf->hd_reg, 0x3 << ch_conf->hd_shift,
				    hd_audio << ch_conf->hd_shift);

	return 0;
}
static int afe_memif_set_params(const struct device *dev, uint32_t chan_id, unsigned int channels,
				unsigned int rate, unsigned int format)
{
	int ret;

	ret = afe_memif_set_channels(dev, chan_id, channels);
	if (ret < 0)
		return ret;
	ret = afe_memif_set_rate(dev, chan_id, rate);
	if (ret < 0)
		return ret;
	ret = afe_memif_set_format(dev, chan_id, format);
	if (ret < 0)
		return ret;
	/* TODO IRQ direction, irq format setting ? */

	return ret;
}
static int afe_memif_set_addr(const struct device *dev, uint32_t chan_id, unsigned int dma_addr,
		       unsigned int dma_bytes)
{
	const struct memif_channel_conf *ch_conf = dma_chan_get_conf(dev, chan_id);

	// struct mtk_base_afe_memif *memif = &afe->memif[id];
	int msb_at_bit33 = 0; /* for dsp side only support 32bit address */
	unsigned int phys_buf_addr;
	unsigned int phys_buf_addr_upper_32 = 0; /* for dsp side only support 32bit address */

	// memif->dma_addr = dma_addr;

	/* convert adsp address to afe address */
	// TODO, need convert dma_addr from adsp address to afe address by a look-up table or something else, or just use the original dma_addr if afe and adsp share the same view of memory
	// if (afe->adsp2afe_addr)
	// 	dma_addr = afe->adsp2afe_addr(dma_addr);

	phys_buf_addr = dma_addr;

	// memif->afe_addr = phys_buf_addr;
	// memif->buffer_size = dma_bytes;
	LOG_DBG("dma_addr:0x%x, size:%u\n", dma_addr, dma_bytes);
	/* start */
	afe_reg_write(ch_conf->reg_ofs_base, phys_buf_addr);
	/* end */
	if (ch_conf->reg_ofs_end)
		afe_reg_write(ch_conf->reg_ofs_end, phys_buf_addr + dma_bytes - 1);
	// else
	// 	afe_reg_write(ch_conf->reg_ofs_base + afe->base_end_offset,
	// 		      phys_buf_addr + dma_bytes - 1);

	/* set start, end, upper 32 bits */
	if (ch_conf->reg_ofs_base_msb) {
		afe_reg_write(ch_conf->reg_ofs_base_msb, phys_buf_addr_upper_32);
		afe_reg_write(ch_conf->reg_ofs_end_msb, phys_buf_addr_upper_32);
	}

	/* set MSB to 33-bit */
	if (ch_conf->msb_reg > 0)
		afe_reg_update_bits(ch_conf->msb_reg, 1 << ch_conf->msb_shift,
				    msb_at_bit33 << ch_conf->msb_shift);

	/* set MSB to 33-bit, for memif end address */
	if (ch_conf->msb2_reg > 0)
		afe_reg_update_bits(ch_conf->msb2_reg, 1 << ch_conf->msb2_shift,
				    msb_at_bit33 << ch_conf->msb2_shift);

	return 0;
}

static int afe_memif_set_enable(const struct device *dev, uint32_t chan_id, uint32_t enable)
{
	const struct memif_channel_conf *ch_conf = dma_chan_get_conf(dev, chan_id);

	if (ch_conf->enable_shift < 0)
		return 0;

	/* enable agent */
	/* TODO: enable/disable should in different sequence? */
	if (ch_conf->agent_disable_reg > 0) {
		afe_reg_update_bits(ch_conf->agent_disable_reg,
				    1 << ch_conf->agent_disable_shift,
				    (!enable) << ch_conf->agent_disable_shift);
	}

	afe_reg_update_bits(ch_conf->enable_reg, 1 << ch_conf->enable_shift,
			    enable << ch_conf->enable_shift);

	return 0;
}
static unsigned int afe_memif_get_cur_position(const struct device *dev, uint32_t chan_id)
{
	const struct memif_channel_conf *ch_conf = dma_chan_get_conf(dev, chan_id);
	unsigned int hw_ptr = 0;

	if (ch_conf->reg_ofs_cur < 0)
		return 0;

	afe_reg_read(ch_conf->reg_ofs_cur, &hw_ptr);

	/* convert afe address to adsp address */
	// if (ch_conf->afe2adsp_addr)
	// 	hw_ptr = ch_conf->afe2adsp_addr(hw_ptr);

	return hw_ptr;
}

//-----------------------------------------------------------------------------------------
static int dma_mtk_memif_config(const struct device *dev, uint32_t chan_id,
		       struct dma_config *dma_cfg)
{
	const struct memif_channel_conf *ch_conf = dma_chan_get_conf(dev, chan_id);
	struct memif_channel_data *ch_data = dma_chan_get_data(dev, chan_id);
	struct memif_dai_config *dai_conf;

	// unsigned int dma_addr;
	// int i, dai_id, irq_id, direction
	int ret;
	struct dma_block_config *next_block;
	uint32_t dma_size = 0;

	if (!dma_cfg->user_data) {
		LOG_ERR("memif_dai_conf shouldn't be NULL");
		return -EINVAL;
	}

	if (!dma_cfg->cyclic) {
		LOG_ERR("Only cyclic configurations are supported!");
		return -ENOTSUP;
	}

	if (!dma_cfg->head_block) {
		LOG_ERR("head block shouldn't be NULL");
		return -EINVAL;
	}

	/* Scatter-Gather configurations currently not supported */
	if (dma_cfg->block_count != 1) {
		LOG_ERR("number of blocks %d not supported", dma_cfg->block_count);
		return -ENOTSUP;
	}

	// channel->is_scheduling_source = config->is_scheduling_source;

	switch (dma_cfg->channel_direction) {
	case MEMORY_TO_PERIPHERAL:
		if (!ch_conf->downlink)
			return -EINVAL;

		// dai_id = (int)AFE_HS_GET_DAI(config->dest_dev);
		// irq_id = (int)AFE_HS_GET_IRQ(config->dest_dev);

		/* source address shouldn't be NULL */
		if (!dma_cfg->head_block->source_address) {
			LOG_ERR("source address cannot be NULL");
			return -EINVAL;
		}

		ch_data->dma_base = (int)dma_cfg->head_block->source_address;
		break;
	case PERIPHERAL_TO_MEMORY:
		if (ch_conf->downlink)
			return -EINVAL;

		// dai_id = (int)AFE_HS_GET_DAI(config->src_dev);
		// irq_id = (int)AFE_HS_GET_IRQ(config->src_dev);

		/* destination address shouldn't be NULL */
		if (!dma_cfg->head_block->dest_address) {
			LOG_ERR("destination address cannot be NULL");
			return -EINVAL;
		}

		ch_data->dma_base = (int)dma_cfg->head_block->dest_address;
		break;
	default:
		LOG_ERR("%s: unsupported config direction", __func__);
		return -EINVAL;
	}

	dma_size = dma_cfg->head_block->block_size;
	while((next_block = dma_cfg->head_block->next_block)) {
		dma_size += next_block->block_size;
	}
	// for (i = 0; i < dma_cfg->block_count; i++)
	// 	dma_size += (int)dma_cfg->head_block->block_size;

	// if (dma_cfg->scatter) {
	// 	LOG_ERR("scatter enabled, that is not supported for now!");
	// 	return -ENOTSUP;
	// }

	// ch_data->dai_id = dai_id;
	// memif->irq_id = irq_id;
	// memif->direction = direction;
	ch_data->dma_size = dma_size;

	/* TODO risk, it may has sync problems with DAI comp */
	ch_data->rptr = 0;
	ch_data->wptr = 0;
	ch_data->period_size = dma_cfg->head_block->block_size;

	/* get dai's config setting from afe driver */
	// ret = afe_dai_get_config(memif->afe, dai_id, &ch_data->channels, &ch_data->rate, &ch_data->format);
	// if (ret < 0)
	// 	return ret;

	/* get dai's config setting from user_data */
	dai_conf = (struct memif_dai_config *)dma_cfg->user_data;
	ch_data->channels = dai_conf->channels;
	ch_data->fs = dai_conf->fs;

	/* memif format should follow DAI component, not dai hw configuration */
	switch (dai_conf->src_width) {
	case 2:
		ch_data->format = DAI_FRAME_S16_LE;
		break;
	case 4:
		ch_data->format = DAI_FRAME_S32_LE;
		break;
	default:
		LOG_ERR("not support bitwidth %u!", dai_conf->src_width);
		return -ENOTSUP;
	}

	/* set the afe memif parameters */
	ret = afe_memif_set_params(dev, chan_id, ch_data->channels, ch_data->fs,
				   ch_data->format);
	if (ret < 0)
		return ret;
	ret = afe_memif_set_addr(dev, chan_id, ch_data->dma_base, ch_data->dma_size);
	if (ret < 0)
		return ret;
	ch_data->status = COMP_STATE_PREPARE;

	return 0;
}

static int dma_mtk_memif_get_status(const struct device *dev, uint32_t chan_id,
			   struct dma_status *stat)
{
	struct memif_channel_data *ch_data = dma_chan_get_data(dev, chan_id);

	stat->read_position = ch_data->rptr + ch_data->dma_base;
	stat->write_position = ch_data->wptr + ch_data->dma_base;
	stat->free = ch_data->free;
	stat->pending_length = ch_data->pending_length;

	return 0;
}

static int dma_mtk_memif_suspend(const struct device *dev, uint32_t chan_id)
{
	struct memif_channel_data *ch_data = dma_chan_get_data(dev, chan_id);

	/* TODO actually handle pause/release properly? */
	if (ch_data->status != COMP_STATE_ACTIVE)
		return -EINVAL;

	ch_data->status = COMP_STATE_PAUSED;

	/* Disable HW requests */
	return afe_memif_set_enable(dev, chan_id, 0);
}

static int dma_mtk_memif_resume(const struct device *dev, uint32_t chan_id)
{
	struct memif_channel_data *ch_data = dma_chan_get_data(dev, chan_id);

	/* TODO actually handle pause/release properly? */
	if (ch_data->status != COMP_STATE_PAUSED)
		return -EINVAL;

	ch_data->status = COMP_STATE_ACTIVE;

	/* Disable HW requests */
	return afe_memif_set_enable(dev, chan_id, 1);
}

static int dma_mtk_memif_stop(const struct device *dev, uint32_t chan_id)
{
	struct memif_channel_data *ch_data = dma_chan_get_data(dev, chan_id);
	
	/* Validate state */
	/* TODO: Should we? */
	switch (ch_data->status) {
	case COMP_STATE_READY:
	case COMP_STATE_PREPARE:
		return 0; /* do not try to stop multiple times */
	case COMP_STATE_PAUSED:
	case COMP_STATE_ACTIVE:
		break;
	default:
		return -EINVAL;
	}
	ch_data->status = COMP_STATE_READY;

#if CONFIG_TEST_SGEN
	afe_sinegen_disable();
#endif

	/* Disable channel */
	return afe_memif_set_enable(dev, chan_id, 0);
}

static int dma_mtk_memif_start(const struct device *dev, uint32_t chan_id)
{
	struct memif_channel_data *ch_data = dma_chan_get_data(dev, chan_id);

 	if (ch_data->status != COMP_STATE_PREPARE) //data->status != COMP_STATE_SUSPEND)
 		return -EINVAL;

	ch_data->status = COMP_STATE_ACTIVE;

#if CONFIG_TEST_SGEN
	afe_sinegen_enable();
#endif

/* Do the HW start of the DMA */
	return afe_memif_set_enable(dev, chan_id, 1);
}

static int dma_mtk_memif_reload(const struct device *dev, uint32_t chan_id, uint32_t src,
		       uint32_t dst, size_t size)
{
	struct memif_channel_data *ch_data = dma_chan_get_data(dev, chan_id);
	const struct memif_channel_conf *ch_conf = dma_chan_get_conf(dev, chan_id);
	unsigned int hw_ptr;

	/* update current hw point */
	hw_ptr = afe_memif_get_cur_position(dev, chan_id);

	if (!hw_ptr)
		return -EINVAL;

	hw_ptr -= ch_data->dma_base;

	if (ch_conf->downlink)
		ch_data->rptr = hw_ptr;
	else
		ch_data->wptr = hw_ptr;

	ch_data->pending_length = (ch_data->wptr + ch_data->dma_size - ch_data->rptr) % ch_data->dma_size;

	/* TODO, check if need alignment the available and free size to 1 period */
	if (ch_conf->downlink)
		ch_data->pending_length = DIV_ROUND_UP(ch_data->pending_length, ch_data->period_size) * ch_data->period_size;
	else
		ch_data->pending_length = ch_data->pending_length / ch_data->period_size * ch_data->period_size;

	ch_data->free = ch_data->dma_size - ch_data->pending_length;

	return 0;
}

int dma_mtk_memif_get_attribute(const struct device *dev, uint32_t type, uint32_t *value)
{
	switch (type) {
	case DMA_ATTR_BUFFER_ADDRESS_ALIGNMENT:
		*value = DMA_BUF_ADDR_ALIGNMENT(
				DT_COMPAT_GET_ANY_STATUS_OKAY(mediatek_memif));
		break;
	case DMA_ATTR_BUFFER_SIZE_ALIGNMENT:
		*value = DMA_BUF_SIZE_ALIGNMENT(
				DT_COMPAT_GET_ANY_STATUS_OKAY(mediatek_memif));
		break;
	case DMA_ATTR_COPY_ALIGNMENT:
		*value = DMA_COPY_ALIGNMENT(
				DT_COMPAT_GET_ANY_STATUS_OKAY(mediatek_memif));
		break;
	case DMA_ATTR_MAX_BLOCK_COUNT:
		*value = 4;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static bool dma_mtk_memif_channel_filter(const struct device *dev, int chan_id, void *param)
{
	struct memif_channel_data *ch_data = dma_chan_get_data(dev, chan_id);

	if (!param)
		return false;

	if (*(int *)param != chan_id)
		return false;

	if (ch_data->status != COMP_STATE_INIT) {
		LOG_ERR("Cannot reuse channel %d", chan_id);
		return false;
	}

	ch_data->status = COMP_STATE_READY;

	return true;
}

static void dma_mtk_memif_channel_release(const struct device *dev, uint32_t chan_id)
{
	struct memif_channel_data *ch_data = dma_chan_get_data(dev, chan_id);

	// notifier_unregister_all(NULL, channel);
	ch_data->status = COMP_STATE_INIT;

	return true;
}

static DEVICE_API(dma, memif_api) = {
	.reload = dma_mtk_memif_reload,
	.config = dma_mtk_memif_config,
	.start = dma_mtk_memif_start,
	.stop = dma_mtk_memif_stop,
	.suspend = dma_mtk_memif_suspend,
	.resume = dma_mtk_memif_resume,
	.get_status = dma_mtk_memif_get_status,
	.get_attribute = dma_mtk_memif_get_attribute,
	.chan_filter = dma_mtk_memif_channel_filter,
	.chan_release = dma_mtk_memif_channel_release,
};

// int memif_pm_action(const struct device *dev, enum pm_device_action action)
// {
// 	switch (action) {
// 	case PM_DEVICE_ACTION_RESUME:
// 		// memif_enable_afe(dev); //TODO	enable AFE here ???????
// 		break;
// 	case PM_DEVICE_ACTION_SUSPEND:
// 	case PM_DEVICE_ACTION_TURN_ON:
// 	case PM_DEVICE_ACTION_TURN_OFF:
// 		break;
// 	default:
// 		return -ENOTSUP;
// 	}

// 	return 0;
// }

// int memif_init(const struct device *dev)
// {
// 	const struct memif_conf *conf = dev->config;
// 	struct memif_data *data = dev->data;

// 	data->ch_atomic = ATOMIC_INIT(0);
// 	data->ctx.atomic = data->ch_atomic;
// 	data->ctx.dma_channels = conf->ch_cnt;
// 	data->ctx.magic = DMA_MAGIC;
// 	//memif_channels_init(dev);

// 	return 0;
	
// 	// pm_device_driver_init(dev, memif_pm_action);
// }


#define MEMIF_CHANNEL_INIT(node_id) \
		{ \
			.id = DT_PROP(node_id, dai_id), \
			.name = DT_PROP(node_id, afe_name), \
			.downlink = DT_PROP(node_id, downlink), \
			.reg_ofs_base = DT_PROP_BY_IDX(node_id, base, 1), \
			.reg_ofs_cur = DT_PROP_BY_IDX(node_id, cur, 1), \
			.reg_ofs_end = DT_PROP_BY_IDX(node_id, end, 1), \
			.reg_ofs_base_msb = DT_PROP_BY_IDX(node_id, base, 0), \
			.reg_ofs_cur_msb = DT_PROP_BY_IDX(node_id, cur, 0), \
			.reg_ofs_end_msb = DT_PROP_BY_IDX(node_id, end, 0), \
			.msb_reg = DT_PROP_BY_IDX(node_id, msb, 0), \
			.msb_shift = DT_PROP_BY_IDX(node_id, msb, 1), \
			.msb2_reg = DT_PROP_BY_IDX(node_id, msb2, 0), \
			.msb2_shift = DT_PROP_BY_IDX(node_id, msb2, 1), \
			.ch_num_reg = DT_PROP_BY_IDX(node_id, ch_num, 0), \
			.ch_num_shift = DT_PROP_BY_IDX(node_id, ch_num, 1), \
			.ch_num_maskbit = BIT(DT_PROP_BY_IDX(node_id, ch_num, 2)) - 1, \
			.hd_reg = DT_PROP_BY_IDX(node_id, hd, 0), \
			.hd_shift = DT_PROP_BY_IDX(node_id, hd, 1), \
			.fs_reg = DT_PROP_BY_IDX(node_id, fs, 0), \
			.fs_shift = DT_PROP_BY_IDX(node_id, fs, 1), \
			.fs_maskbit = BIT(DT_PROP_BY_IDX(node_id, fs, 2)) - 1, \
			.enable_reg = DT_PROP_BY_IDX(node_id, enable, 0), \
			.enable_shift = DT_PROP_BY_IDX(node_id, enable, 1), \
			.agent_disable_reg = DT_PROP_BY_IDX(node_id, agent_disable, 0), \
			.agent_disable_shift = DT_PROP_BY_IDX(node_id, agent_disable, 1), \
		}, 


//.base = DT_REG_ADDR(DT_DRV_INST(inst)),

#define MEMIF_INIT(inst) \
	\
	static const struct memif_conf memif_##inst##_config = { \
		.fs2reg_cnt = DT_PROP_LEN(DT_DRV_INST(inst), fs2reg) / 2, \
		.fs2reg_array = DT_PROP(DT_DRV_INST(inst), fs2reg), \
		.ch_cnt = DT_CHILD_NUM_STATUS_OKAY(DT_DRV_INST(inst)), \
		.ch_conf = { \
			DT_INST_FOREACH_CHILD_STATUS_OKAY(inst, MEMIF_CHANNEL_INIT) \
		}, \
	}; \
	\
	static ATOMIC_DEFINE(dma_channels_atomic_##inst, MEMIF_CHANNELS_COUNT);	\
	\
	static struct memif_data memif_##inst##_data = { \
		.ctx = { \
			.magic = DMA_MAGIC, \
			.dma_channels = MEMIF_CHANNELS_COUNT, \
			.atomic = dma_channels_atomic_##inst, \
		}, \
		.ch_data = {{0}}, \
	}; \
	\
	static int memif_##inst##_init(const struct device *dev) \
	{ \
		return 0; \
	} \
	\
	DEVICE_DT_INST_DEFINE(inst, memif_##inst##_init, \
		    NULL, \
		    &memif_##inst##_data, \
		    &memif_##inst##_config, POST_KERNEL, \
		    CONFIG_DMA_INIT_PRIORITY, \
		    &memif_api);

DT_INST_FOREACH_STATUS_OKAY(MEMIF_INIT)

//PM_DEVICE_DT_INST_DEFINE(inst, memif_channel_pm_action);
//		.downlink = DT_INST_PROP(inst, downlink),
	

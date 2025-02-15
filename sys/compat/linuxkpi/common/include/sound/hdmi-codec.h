#ifndef _LINUXKPI_SOUND_HDMI_CODEC_H_
#define _LINUXKPI_SOUND_HDMI_CODEC_H_

struct device;
struct hdmi_codec_daifmt;

#define HDMI_CODEC_DRV_NAME "hdmi-codec"

struct hdmi_codec_ops {
	int (*audio_startup)(struct device *, void *);
	int (*prepare)(struct device *, void *, struct hdmi_codec_daifmt *, struct hdmi_codec_params *);
	void (*audio_shutdown)(struct device *, void *);
	int (*mute_stream)(struct device *, void *, bool, int);
	int (*get_eld)(struct device *, void *, uint8_t *, size_t);
};

struct hdmi_codec_pdata {
	const struct hdmi_codec_ops *ops;
	unsigned int max_i2s_channels;
	bool i2s;
	bool spdif;
	bool no_i2s_capture;
	bool no_spdif_capture;
	struct drm_connector *data;
};

#endif

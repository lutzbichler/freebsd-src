/* Public domain */

#ifndef LINUXKPI_ACPI_AMD_WBRF_H_
#define LINUXKPI_ACPI_AMD_WBRF_H_

#define	MAX_NUM_OF_WBRF_RANGES	11
#define	WBRF_CHANGED		0

struct freq_band_range {
	uint64_t	start;
	uint64_t	end;
};

struct wbrf_ranges_in_out {
	struct freq_band_range	*band_list;
};

static inline int
acpi_amd_wbrf_supported_consumer(struct device *dev __unused)
{
	return (0);
}

static inline int
amd_wbrf_retrieve_freq_band(struct device *dev __unused,
	struct wbrf_ranges_in_out *ranges __unused)
{
	return (0);
}

static inline int
amd_wbrf_register_notifier(struct notifier_block *notifier __unused)
{
	return (0);
}

static inline int
amd_wbrf_unregister_notifier(struct notifier_block *notifier __unused)
{
	return (0);
}

#endif /* LINUXKPI_ACPI_AMD_WBRF_H_ */

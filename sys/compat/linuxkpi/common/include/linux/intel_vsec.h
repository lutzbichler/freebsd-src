/* Public domain */

#ifndef _LINUXKPI_INTEL_VSEC_H_
#define _LINUXKPI_INTEL_VSEC_H_

struct pci_dev;

#define	VSEC_ID_TELEMETRY	0x01
#define	VSEC_ID_CRASHLOG	0x02

#define VSEC_CAP_TELEMETRY	0x01
#define VSEC_CAP_CRASHLOG	0x02

struct intel_vsec_header {
	uint8_t rev;
	uint16_t id;
	uint16_t length;
	uint8_t entry_size;
	uint8_t num_entries;
	uint8_t tbir;
	uint32_t offset;
};

struct intel_vsec_platform_info {
	uint16_t caps;
	struct intel_vsec_header **headers;
	void *priv_data;
};

struct pmt_callbacks {
	int (*read_telem)(struct pci_dev *pdev, u32 guid, u64 *data, loff_t off, u32 count);
};

static void
intel_vsec_register(struct pci_dev *pdev, struct intel_vsec_platform_info *info)
{
}

#endif /* _LINUXKPI_INTEL_VSEC_H_ */

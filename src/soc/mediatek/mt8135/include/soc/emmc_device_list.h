#ifndef SOC_MEDIATEK_MT8135_EMMC_DEVICE_LIST_H
#define SOC_MEDIATEK_MT8135_EMMC_DEVICE_LIST_H

#include <stdint.h>

struct mmcdev_info {
	/* MID + PNM in CID register */
	u8 m_id;		/* Manufacturer ID */
	char pro_name[8];	/* Product name */

	u8 r_smpl;
	u8 d_smpl;
	u8 cmd_rxdly;
	u8 rd_rxdly;
	u8 wr_rxdly;
};

static const struct mmcdev_info g_mmcTable[] = {
	/* hynix */
	{0x90, "HYNIX ", 0, 0, 0, 0, 0},

	/* end */
	{0x00, "xxxxxx", 0, 0, 0, 0, 0}
};

#endif /* SOC_MEDIATEK_MT8135_EMMC_DEVICE_LIST_H */

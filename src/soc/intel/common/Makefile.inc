subdirs-$(CONFIG_COMMON_BOOTBLOCK) += bootblock

romstage-$(CONFIG_CACHE_MRC_SETTINGS) += mrc_cache.c
romstage-$(CONFIG_COMMON_RAM_INIT) += raminit.c
romstage-$(CONFIG_COMMON_RESET) += reset.c
romstage-$(CONFIG_COMMON_ROMSTAGE) += romstage.c

ramstage-y += hda_verb.c
ramstage-$(CONFIG_CACHE_MRC_SETTINGS) += nvm.c
ramstage-$(CONFIG_CACHE_MRC_SETTINGS) += mrc_cache.c
ramstage-$(CONFIG_COMMON_RESET) += reset.c

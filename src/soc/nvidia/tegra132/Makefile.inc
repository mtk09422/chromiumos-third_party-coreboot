CBOOTIMAGE = cbootimage

bootblock-y += bootblock.c
bootblock-y += bootblock_asm.S
bootblock-y += cbfs.c
bootblock-y += timer.c
bootblock-y += clock.c
bootblock-y += ../tegra/gpio.c
bootblock-y += ../tegra/i2c.c
bootblock-y += ../tegra/pingroup.c
bootblock-y += ../tegra/pinmux.c
bootblock-y += ../tegra/apbmisc.c
ifeq ($(CONFIG_BOOTBLOCK_CONSOLE),y)
bootblock-$(CONFIG_CONSOLE_SERIAL_UART) += uart.c
endif

romstage-y += cbfs.c
romstage-y += timer.c
romstage-y += ../tegra/gpio.c
romstage-y += ../tegra/i2c.c
romstage-y += ../tegra/pinmux.c
ifeq ($(CONFIG_EARLY_CONSOLE),y)
romstage-$(CONFIG_CONSOLE_SERIAL_UART) += uart.c
endif

ramstage-y += cbfs.c
ramstage-y += timer.c
ramstage-y += ../tegra/gpio.c
ramstage-y += ../tegra/i2c.c
ramstage-y += ../tegra/pinmux.c
ramstage-$(CONFIG_CONSOLE_SERIAL_UART) += uart.c

INCLUDES += -Isrc/soc/nvidia/tegra132/include/

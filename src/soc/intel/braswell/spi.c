/*
 * Copyright (c) 2013 Google Inc.
 * Copyright (C) 2015 Intel Corp.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* This file is derived from the flashrom project. */
#include <arch/io.h>
#include <bootstate.h>
#include <console/console.h>
#include <delay.h>
#include <device/pci_ids.h>
#include <soc/lpc.h>
#include <soc/pci_devs.h>
#include <spi_flash.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __SMM__
#define pci_read_config_byte(dev, reg, targ)\
	*(targ) = pci_read_config8(dev, reg)
#define pci_read_config_word(dev, reg, targ)\
	*(targ) = pci_read_config16(dev, reg)
#define pci_read_config_dword(dev, reg, targ)\
	*(targ) = pci_read_config32(dev, reg)
#define pci_write_config_byte(dev, reg, val)\
	pci_write_config8(dev, reg, val)
#define pci_write_config_word(dev, reg, val)\
	pci_write_config16(dev, reg, val)
#define pci_write_config_dword(dev, reg, val)\
	pci_write_config32(dev, reg, val)
#else /* !__SMM__ */
#include <device/device.h>
#include <device/pci.h>
#define pci_read_config_byte(dev, reg, targ)\
	*(targ) = pci_read_config8(dev, reg)
#define pci_read_config_word(dev, reg, targ)\
	*(targ) = pci_read_config16(dev, reg)
#define pci_read_config_dword(dev, reg, targ)\
	*(targ) = pci_read_config32(dev, reg)
#define pci_write_config_byte(dev, reg, val)\
	pci_write_config8(dev, reg, val)
#define pci_write_config_word(dev, reg, val)\
	pci_write_config16(dev, reg, val)
#define pci_write_config_dword(dev, reg, val)\
	pci_write_config32(dev, reg, val)
#endif /* !__SMM__ */

typedef struct spi_slave ich_spi_slave;

static int ichspi_lock = 0;

typedef struct ich9_spi_regs {
	uint32_t bfpr;
	uint16_t hsfs;
	uint16_t hsfc;
	uint32_t faddr;
	uint32_t _reserved0;
	uint32_t fdata[16];
	uint32_t frap;
	uint32_t freg[5];
	uint32_t _reserved1[3];
	uint32_t pr[5];
	uint32_t _reserved2[2];
	uint8_t ssfs;
	uint8_t ssfc[3];
	uint16_t preop;
	uint16_t optype;
	uint8_t opmenu[8];
} __attribute__((packed)) ich9_spi_regs;

typedef struct ich_spi_controller {
	int locked;

	uint8_t *opmenu;
	int menubytes;
	uint16_t *preop;
	uint16_t *optype;
	uint32_t *addr;
	uint8_t *data;
	unsigned databytes;
	uint8_t *status;
	uint16_t *control;
	uint32_t *bbar;
} ich_spi_controller;

static ich_spi_controller cntlr;

enum {
	SPIS_SCIP =		0x0001,
	SPIS_GRANT =		0x0002,
	SPIS_CDS =		0x0004,
	SPIS_FCERR =		0x0008,
	SSFS_AEL =		0x0010,
	SPIS_LOCK =		0x8000,
	SPIS_RESERVED_MASK =	0x7ff0,
	SSFS_RESERVED_MASK =	0x7fe2
};

enum {
	SPIC_SCGO =		0x000002,
	SPIC_ACS =		0x000004,
	SPIC_SPOP =		0x000008,
	SPIC_DBC =		0x003f00,
	SPIC_DS =		0x004000,
	SPIC_SME =		0x008000,
	SSFC_SCF_MASK =		0x070000,
	SSFC_RESERVED =		0xf80000
};

enum {
	HSFS_FDONE =		0x0001,
	HSFS_FCERR =		0x0002,
	HSFS_AEL =		0x0004,
	HSFS_BERASE_MASK =	0x0018,
	HSFS_BERASE_SHIFT =	3,
	HSFS_SCIP =		0x0020,
	HSFS_FDOPSS =		0x2000,
	HSFS_FDV =		0x4000,
	HSFS_FLOCKDN =		0x8000
};

enum {
	HSFC_FGO =		0x0001,
	HSFC_FCYCLE_MASK =	0x0006,
	HSFC_FCYCLE_SHIFT =	1,
	HSFC_FDBC_MASK =	0x3f00,
	HSFC_FDBC_SHIFT =	8,
	HSFC_FSMIE =		0x8000
};

enum {
	SPI_OPCODE_TYPE_READ_NO_ADDRESS =	0,
	SPI_OPCODE_TYPE_WRITE_NO_ADDRESS =	1,
	SPI_OPCODE_TYPE_READ_WITH_ADDRESS =	2,
	SPI_OPCODE_TYPE_WRITE_WITH_ADDRESS =	3
};

#if IS_ENABLED(CONFIG_DEBUG_SPI_FLASH)

static u8 readb_(void *addr)
{
	u8 v = read8(addr);
	printk(BIOS_DEBUG, "0x%p --> 0x%2.2x\n", addr, v);
	return v;
}

static u16 readw_(void *addr)
{
	u16 v = read16(addr);
	printk(BIOS_DEBUG, "0x%p --> 0x%4.4x\n", addr, v);
	return v;
}

static u32 readl_(void *addr)
{
	u32 v = read32(addr);
	printk(BIOS_DEBUG, "0x%p --> 0x%8.8x\n", addr, v);
	return v;
}

static void writeb_(u8 b, void *addr)
{
	printk(BIOS_DEBUG, "0x%p <-- 0x%2.2x\n", addr, b);
	write8(addr, b);
}

static void writew_(u16 b, void *addr)
{
	printk(BIOS_DEBUG, "0x%p <-- 0x%4.4x\n", addr, b);
	write16(addr, b);
}

static void writel_(u32 b, void *addr)
{
	printk(BIOS_DEBUG, "0x%p <-- 0x%8.8x\n", addr, b);
	write32(addr, b);
}

#else /* CONFIG_DEBUG_SPI_FLASH ^^^ enabled  vvv NOT enabled */

#define readb_(a) read8(a)
#define readw_(a) read16(a)
#define readl_(a) read32(a)
#define writeb_(val, addr) write8(addr, val)
#define writew_(val, addr) write16(addr, val)
#define writel_(val, addr) write32(addr, val)

#endif  /* CONFIG_DEBUG_SPI_FLASH ^^^ NOT enabled */

static void write_reg(const void *value, void *dest, uint32_t size)
{
	const uint8_t *bvalue = value;
	uint8_t *bdest = dest;

	while (size >= 4) {
		writel_(*(const uint32_t *)bvalue, bdest);
		bdest += 4; bvalue += 4; size -= 4;
	}
	while (size) {
		writeb_(*bvalue, bdest);
		bdest++; bvalue++; size--;
	}
}

static void read_reg(void *src, void *value, uint32_t size)
{
	uint8_t *bsrc = src;
	uint8_t *bvalue = value;

	while (size >= 4) {
		*(uint32_t *)bvalue = readl_(bsrc);
		bsrc += 4; bvalue += 4; size -= 4;
	}
	while (size) {
		*bvalue = readb_(bsrc);
		bsrc++; bvalue++; size--;
	}
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs)
{
	ich_spi_slave *slave = malloc(sizeof(*slave));

	if (!slave) {
		printk(BIOS_ERR, "ICH SPI: Bad allocation\n");
		return NULL;
	}

	memset(slave, 0, sizeof(*slave));

	slave->bus = bus;
	slave->cs = cs;
	return slave;
}

static ich9_spi_regs *spi_regs(void)
{
	device_t dev;
	uint32_t sbase;

#ifdef __SMM__
	dev = PCI_DEV(0, LPC_DEV, LPC_FUNC);
#else
	dev = dev_find_slot(0, PCI_DEVFN(LPC_DEV, LPC_FUNC));
#endif
	pci_read_config_dword(dev, SBASE, &sbase);
	sbase &= ~0x1ff;

	return (void *)sbase;
}

void spi_init(void)
{
	ich9_spi_regs *ich9_spi = spi_regs();

#if IS_ENABLED(CONFIG_DEBUG_SPI_FLASH)
	printk(BIOS_DEBUG, "%s/%s\n", __FILE__, __func__);
#endif	/* CONFIG_DEBUG_SPI_FLASH */

	ichspi_lock = readw_(&ich9_spi->hsfs) & HSFS_FLOCKDN;
	cntlr.opmenu = ich9_spi->opmenu;
	cntlr.menubytes = sizeof(ich9_spi->opmenu);
	cntlr.optype = &ich9_spi->optype;
	cntlr.addr = &ich9_spi->faddr;
	cntlr.data = (uint8_t *)ich9_spi->fdata;
	cntlr.databytes = sizeof(ich9_spi->fdata);
	cntlr.status = &ich9_spi->ssfs;
	cntlr.control = (uint16_t *)ich9_spi->ssfc;
	cntlr.preop = &ich9_spi->preop;
}


static void spi_init_cb(void *unused)
{
	spi_init();
}

BOOT_STATE_INIT_ENTRIES(spi_init_bscb) = {
	BOOT_STATE_INIT_ENTRY(BS_DEV_INIT, BS_ON_ENTRY, spi_init_cb, NULL),
};

int spi_claim_bus(struct spi_slave *slave)
{
	/* Handled by ICH automatically. */
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	/* Handled by ICH automatically. */
}

/*
 * Wait for up to 60ms til the controller completes the operation or
 * returns an error.  Clear the completion status.
 *
 * Return the last read status value on success or -1 on failure.
 */
static int ich_status_poll(void)
{
	int timeout = 40000; /* This will result in 400 ms */
	u16 status = 0;

	while (timeout--) {
		status = readw_(cntlr.status);
		if (((status & (SPIS_FCERR | SPIS_CDS | SPIS_SCIP))
			^ SPIS_SCIP) != 0) {
			writew_(SPIS_FCERR | SPIS_CDS, cntlr.status);
			return status;
		}
		udelay(10);
	}

	printk(BIOS_ERR, "ICH SPI: SCIP timeout, read %x, expected %x\n",
		status, (status & (~(SPIS_FCERR | SPIS_SCIP))) | SPIS_CDS);
	return -1;
}

int spi_xfer(struct spi_slave *slave, const void *dout,
		unsigned int bytesout, void *din, unsigned int bytesin)
{
	unsigned int address;
	unsigned int bytes_to_transfer;
	unsigned int control;
	u8 *data;
	unsigned int data_bytes;
	u8 opcode;
	unsigned int opcode_index;
	u8 opcode_type;
	unsigned int prefix_index;
	int status;
	int with_address;
	int write_operation;
	static u8 write_prefix;

#if IS_ENABLED(CONFIG_DEBUG_SPI_FLASH)
	printk(BIOS_DEBUG, "%s/%s (0x%p, 0x%p, 0x%08x, 0x%p, 0x%08x)\n",
			__FILE__, __func__, slave, dout, bytesout,
			din, bytesin);
#endif	/* CONFIG_DEBUG_SPI_FLASH */

	/*
	 * The following hardware is available for shared use when the
	 * controller is not locked:
	 *
	 *	Opcode 0
	 *	Opcode Type 0
	 *	Opcode Prefix 0
	 *
	 * The rest of the controller is reserved for static setup.
	 */

	/* There has to always at least be an opcode. */
	if (!bytesout || !dout) {
		printk(BIOS_ERR, "ICH SPI: No opcode for transfer\n");

		/* Discard the write prefix */
		write_prefix = 0;
		return -1;
	}

	/* Make sure if we read something we have a place to put it. */
	if (bytesin != 0 && !din) {
		printk(BIOS_ERR, "ICH SPI: Read but no target buffer\n");

		/* Discard the write prefix */
		write_prefix = 0;
		return -1;
	}

	/* Get the opcode */
	data = (u8 *)dout;
	opcode = *data++;
	bytesout -= 1;

	/* Determine if this operation contains an address */
	opcode_type = 0;
	with_address = 0;
	address = 0;
	if (bytesout >= 3) {
		/*
		 * Workarounds:
		 *
		 * Erase operation have an address with no data.
		 * For some reason, the controller does not like
		 * this condition and returns an error.  Instead
		 * of loading the address into the address register
		 * the work-around is to send the address as data.
		 *
		 * Page writes with a short amount of data are
		 * failing.  Send the address as data to workaround
		 * this issue.
		 */
		if (bytesin || (bytesout > (cntlr.databytes - 3))) {
			/* Tell the controller about the address */
			opcode_type = 2;
			with_address = 1;
			address = (unsigned int)(*data++) << 16;
			address |= (unsigned int)(*data++) << 8;
			address |= (unsigned int)(*data++);
			bytesout -= 3;
		}
	}

	/* Determine if this is a write operation */
	write_operation = 0;
	if (bytesin == 0) {
		write_operation = 1;
		opcode_type |= 1;

		/*
		 * The upper layers provide the write prefix byte as
		 * a separate transaction which is how it is handled
		 * on the SPI bus.  Note that a SPI transaction starts
		 * with chip select going low for the target device
		 * and ends with chip select going high.
		 *
		 * However, this controller automatically outputs the
		 * prefix byte at the beginning of write operations.
		 * As such, it is necessary to consume the write prefix
		 * byte.
		 */

		if (write_prefix == 0) {
			/* Delay sending the write prefix byte.  */
			write_prefix = opcode;
#if IS_ENABLED(CONFIG_DEBUG_SPI_FLASH)
			printk(BIOS_DEBUG, "Buffering write prefix: 0x%02x\n",
					write_prefix);
#endif	/* CONFIG_DEBUG_SPI_FLASH */
			return 0;
		}
	}

	/*
	 * Check if more output data exists than the controller can handle.
	 * Iterations for writes are not supported here because each SPI
	 * write command needs to be preceded and followed by other SPI
	 * commands, and this sequence is controlled by the SPI chip driver.
	 */
	if (bytesout > cntlr.databytes) {
		printk(BIOS_ERR,
			"ICH SPI: Too much to write. Does your SPI chip driver use CONTROLLER_PAGE_LIMIT?\n");

		/* Discard the write prefix */
		write_prefix = 0;
		return -1;
	}

	/* Determine the prefix and opcode indices */
	opcode_index = 0;
	prefix_index = 0;
	if (!ichspi_lock) {
		/* Set the write prefix byte */
		if (write_operation) {
			prefix_index = SPIC_ACS;
			writew_(write_prefix, cntlr.preop);
		}

		/* Set the opcode */
		writeb_(opcode, cntlr.opmenu);

		/* Set the opcode type */
		u16 optypes = readw_(cntlr.optype);
		optypes = (optypes & (~3)) | opcode_type;
		writew_(optypes, cntlr.optype);
	} else {
		u8 opmenu[cntlr.menubytes];

		/* Get the opcodes supported by the controller */
		read_reg(cntlr.opmenu, opmenu, cntlr.menubytes);

		/* Verify that the locked controller supports this opcode */
		for (;;) {
			if (opcode == opmenu[opcode_index])
				break;
			opcode_index++;
			if (opcode_index > cntlr.menubytes) {
				printk(BIOS_ERR,
					"ERROR - SPI transaction failed,"
					" opcode map full!\n");

				/* Discard the write prefix */
				write_prefix = 0;
				return -1;
			}
		}

		/* Get the controller write prefix bytes */
		if (write_operation) {
			u16 prefix_bytes = readw_(cntlr.optype);

			/*
			 * Verify that the locked controller contains the write
			 * prefix byte
			 */
			prefix_index = SPIC_ACS;
			if (write_prefix != (prefix_bytes & 0xff)) {
				prefix_index |= SPIC_SPOP;
				if (write_prefix != (prefix_bytes >> 8)) {
					printk(BIOS_ERR,
						"ERROR - SPI transaction failed"
						", prefix not available!\n");

					/* Discard the write prefix */
					write_prefix = 0;
					return -1;
				}
			}
		}
	}

	/* Determine the transfer length */
	data_bytes = bytesin ? bytesin : bytesout;

	/* Clear any previous error */
	writeb_(SPIS_FCERR | SPIS_CDS, cntlr.status);

	/* Segment the transfer operation if necessary */
	do {
		/* Determine the operation data length */
		control = SPIC_SCGO | prefix_index | (opcode_index << 4);
		bytes_to_transfer = data_bytes;
		if (bytes_to_transfer > cntlr.databytes)
			bytes_to_transfer = cntlr.databytes;
		if (bytes_to_transfer > 0) {
			control |= SPIC_DS
				| (((bytes_to_transfer - 1) & 0x3f) << 8);
			data_bytes -= bytes_to_transfer;

			/* Output the data for the operation */
			if (write_operation)
				write_reg(data, cntlr.data, bytes_to_transfer);
		}

		/* Set the 24-bit address if necessary */
		if (with_address) {
			writel_(address & 0x00ffffff, cntlr.addr);
			address += bytes_to_transfer;
		}

		/* Start the SPI operation */
		writew_((u16)control, cntlr.control);

		/* Wait for the result */
		status = ich_status_poll();
		if (status == -1) {
			/* Discard the write prefix */
			write_prefix = 0;
			return -1;
		}
		if (status & SPIS_FCERR) {
			printk(BIOS_ERR, "ICH SPI: Transaction error\n");

			/* Discard the write prefix */
			write_prefix = 0;
			return -1;
		}

		/* Place the data read from the SPI flash into the buffer */
		if (bytesin > 0) {
			read_reg(cntlr.data, din, bytes_to_transfer);
			bytesin -= bytes_to_transfer;
			din += bytes_to_transfer;
		}
	} while (data_bytes > 0);

	/* Discard the write prefix */
	write_prefix = 0;

	/* Successful SPI transaction */
	return 0;
}

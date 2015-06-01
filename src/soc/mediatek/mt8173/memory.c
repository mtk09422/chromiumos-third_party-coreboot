/*
 * This file is part of the coreboot project.
 *
 * Copyright 2015 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <arch/io.h>
#include <assert.h>
#include <console/console.h>
#include <soc/addressmap.h>
#include <soc/dramc_register.h>
#include <soc/memory.h>
#include <soc/emi.h>

#define MOD "MEM"

#include <soc/wdt.h>
#include <soc/emi_hw.h>

extern u32 g_ddr_reserve_enable;
extern  u32 g_ddr_reserve_success;

#define DRV_WriteReg32(addr, val)	write32((void *)addr, val)

#if MEM_TEST
int complex_mem_test(unsigned int start, unsigned int len);
#endif

#ifdef DRAMC_DEBUG_FULL_MEMORY_SCAN
int complex_mem_test1(unsigned int start, unsigned int len);
#endif

/*
 * init EMI
 */
void mt_mem_init(void)
{
	unsigned int emi_cona;

	/* DDR reserve mode no need to enable memory & test */
	/* Note: factory reset failed workaround */
	if ((g_ddr_reserve_enable == 1) && (g_ddr_reserve_success == 1)) {
		/* EMI register dummy read. Give clock to EMI APB */
		/* register to avoid DRAM access hang... */
		emi_cona = *(volatile unsigned *)(EMI_CONA);
		printk(BIOS_DEBUG, "[DDR Reserve mode] EMI dummy "
					"read CONA = 0x%x\n", emi_cona);
		/* Reset EMI MPU protect setting - otherwise we */
		/* can not use protected region during boot-up time */
		DRV_WriteReg32(EMI_MPUA, 0x0);
		DRV_WriteReg32(EMI_MPUB, 0x0);
		DRV_WriteReg32(EMI_MPUC, 0x0);
		DRV_WriteReg32(EMI_MPUD, 0x0);
		DRV_WriteReg32(EMI_MPUE, 0x0);
		DRV_WriteReg32(EMI_MPUF, 0x0);
		DRV_WriteReg32(EMI_MPUG, 0x0);
		DRV_WriteReg32(EMI_MPUH, 0x0);
		DRV_WriteReg32(EMI_MPUI, 0x0);
		DRV_WriteReg32(EMI_MPUJ, 0x0);
		DRV_WriteReg32(EMI_MPUK, 0x0);
		DRV_WriteReg32(EMI_MPUL, 0x0);
	} else { /* normal boot */
#ifdef DRAMC_DEBUG_RETRY
		U32 i, j, n;

		i = 0x00;
		n = 0x00;
		for (j = 0; j < 3; j++) {
			mt_set_emi();
#if MEM_TEST
			i = complex_mem_test(0x40000000, MEM_TEST_SIZE);
			if (i == 0) {
				n = 0x00;
				printk(BIOS_DEBUG, "[%s] ", MOD);
				printk(BIOS_DEBUG, "complex R/W mem "
							"test pass\n");
			} else {
				n = 0x01;
				printk(BIOS_DEBUG, "[%s] ", MOD);
				printk(BIOS_DEBUG, "complex R/W mem "
							"test fail :%x\n", i);
			}

			if (n) {
				printk(BIOS_DEBUG, "[%s] ", MOD);
				printk(BIOS_DEBUG, "mem test fail, "
							"retry %d ...\n", j);
				continue;
			}
#endif
			break;
		}

		if ((n) && (j >= 3)) {
			/* retry 3 times stiil fail, reboot */
			printk(BIOS_DEBUG, "[%s] ", MOD);
			printk(BIOS_DEBUG, "mem test retry 3 tiems "
						"still fail : reboot\n");
			mtk_arch_reset(1);
			while (1);
		} else {
			/* memory test pass */

		}
#else
		mt_set_emi();

#if MEM_TEST
		{
			int i = 0;

			i = complex_mem_test(0x40000000, MEM_TEST_SIZE);
			if (i == 0) {
				printk(BIOS_DEBUG, "[%s] ", MOD);
				printk(BIOS_DEBUG, "complex R/W mem "
							"test pass\n");
			} else {
				printk(BIOS_DEBUG, "[%s] ", MOD);
				printk(BIOS_DEBUG, "complex R/W mem "
							"test fail :%x\n", i);
				ASSERT(0);
			}
		}
#endif
#endif

	}

#ifdef DDR_RESERVE_MODE
	/* Always enable DDR-reserve mode */
	rgu_dram_reserved(1);
#endif

	/* ============================================= */
#ifdef DRAMC_DEBUG_FULL_MEMORY_SCAN
	{
		int ii = 0;

		ii = complex_mem_test1(0x40000000, 0x8000000);
		if (ii == 0) {
			printk(BIOS_DEBUG, "[%s] A Channel ", MOD);
			printk(BIOS_DEBUG, "complex R/W mem test pass\n");
			printk(BIOS_DEBUG, "===================");
			printk(BIOS_DEBUG, "====================\n");
		} else {
			printk(BIOS_DEBUG, "[%s] A Channel ", MOD);
			printk(BIOS_DEBUG, "complex R/W mem test fail :");
			printk(BIOS_DEBUG, "%x\n", ii);
			printk(BIOS_DEBUG, "===================");
			printk(BIOS_DEBUG, "====================\n");
		}

		uintptr_t addr = EMI_APB_BASE + 0x00000000;

		if (((*(volatile unsigned *)(addr)) & 0x1) == 0x1) {
			/* dual channel */
			ii = complex_mem_test1(0x80000000, 0x8000000);
			if (ii == 0) {
				printk(BIOS_DEBUG, "[%s] B Channel ", MOD);
				printk(BIOS_DEBUG, "complex R/W mem "
							"test pass\n");
				printk(BIOS_DEBUG, "===================");
				printk(BIOS_DEBUG, "====================\n");
			} else {
				printk(BIOS_DEBUG, "[%s] B Channel ", MOD);
				printk(BIOS_DEBUG, "complex R/W mem "
							"test fail :");
				printk(BIOS_DEBUG, "%x\n", ii);
				printk(BIOS_DEBUG, "===================");
				printk(BIOS_DEBUG, "====================\n");
			}
		}
	}
#endif
	/* ============================================= */

}

#if MEM_TEST
/*
 * do memory test
 */
enum {
	PATTERN1 = 0x5A5A5A5A,
	PATTERN2 = 0xA5A5A5A5
};

int complex_mem_test(unsigned int start, unsigned int len)
{
	unsigned char *MEM8_BASE = (unsigned char *)(uintptr_t)start;
	unsigned short *MEM16_BASE = (unsigned short *)(uintptr_t)start;
	unsigned int *MEM32_BASE = (unsigned int *)(uintptr_t)start;
	unsigned int *MEM_BASE = (unsigned int *)(uintptr_t)start;
	unsigned char pattern8;
	unsigned short pattern16;
	unsigned int i, j, size, pattern32;
	unsigned int value;

	size = len >> 2;

	/*  Verify the tied bits (tied high)  */
	for (i = 0; i < size; i++) {
		MEM32_BASE[i] = 0;
	}

	for (i = 0; i < size; i++) {
		if (MEM32_BASE[i] != 0) {
			return -1;
		} else {
			MEM32_BASE[i] = 0xffffffff;
		}
	}

	/*  Verify the tied bits (tied low)  */
	for (i = 0; i < size; i++) {
		if (MEM32_BASE[i] != 0xffffffff) {
			return -2;
		} else
			MEM32_BASE[i] = 0x00;
	}

	/*  Verify pattern 1 (0x00~0xff)  */
	pattern8 = 0x00;
	for (i = 0; i < len; i++)
		MEM8_BASE[i] = pattern8++;
	pattern8 = 0x00;
	for (i = 0; i < len; i++) {
		if (MEM8_BASE[i] != pattern8++) {
			return -3;
		}
	}

	/*  Verify pattern 2 (0x00~0xff)  */
	pattern8 = 0x00;
	for (i = j = 0; i < len; i += 2, j++) {
		if (MEM8_BASE[i] == pattern8)
			MEM16_BASE[j] = pattern8;
		if (MEM16_BASE[j] != pattern8) {
			return -4;
		}
		pattern8 += 2;
	}

	/*  Verify pattern 3 (0x00~0xffff)  */
	pattern16 = 0x00;
	for (i = 0; i < (len >> 1); i++)
		MEM16_BASE[i] = pattern16++;
	pattern16 = 0x00;
	for (i = 0; i < (len >> 1); i++) {
		if (MEM16_BASE[i] != pattern16++) {
			return -5;
		}
	}

	/*  Verify pattern 4 (0x00~0xffffffff)  */
	pattern32 = 0x00;
	for (i = 0; i < (len >> 2); i++)
		MEM32_BASE[i] = pattern32++;
	pattern32 = 0x00;
	for (i = 0; i < (len >> 2); i++) {
		if (MEM32_BASE[i] != pattern32++) {
			return -6;
		}
	}

	/*  Pattern 5: Filling memory range with 0x44332211  */
	for (i = 0; i < size; i++)
		MEM32_BASE[i] = 0x44332211;

	/*  Read Check then Fill Memory with a5a5a5a5 Pattern  */
	for (i = 0; i < size; i++) {
		if (MEM32_BASE[i] != 0x44332211) {
			return -7;
		} else {
			MEM32_BASE[i] = 0xa5a5a5a5;
		}
	}

	/* Read Check then Fill Memory with 00 Byte Pattern at offset 0h */
	for (i = 0; i < size; i++) {
		if (MEM32_BASE[i] != 0xa5a5a5a5) {
			return -8;
		} else {
			MEM8_BASE[i * 4] = 0x00;
		}
	}

	/* Read Check then Fill Memory with 00 Byte Pattern at offset 2h */
	for (i = 0; i < size; i++) {
		if (MEM32_BASE[i] != 0xa5a5a500) {
			return -9;
		} else {
			MEM8_BASE[i * 4 + 2] = 0x00;
		}
	}

	/*  Read Check then Fill Memory with 00 Byte Pattern at offset 1h  */
	for (i = 0; i < size; i++) {
		if (MEM32_BASE[i] != 0xa500a500) {
			return -10;
		} else {
			MEM8_BASE[i * 4 + 1] = 0x00;
		}
	}

	/*  Read Check then Fill Memory with 00 Byte Pattern at offset 3h  */
	for (i = 0; i < size; i++) {
		if (MEM32_BASE[i] != 0xa5000000) {
			return -11;
		} else {
			MEM8_BASE[i * 4 + 3] = 0x00;
		}
	}

	/*  Read Check then Fill Memory with ffff Word Pattern at offset 1h */
	for (i = 0; i < size; i++) {
		if (MEM32_BASE[i] != 0x00000000) {
			return -12;
		} else {
			MEM16_BASE[i * 2 + 1] = 0xffff;
		}
	}

	/*  Read Check then Fill Memory with ffff Word Pattern at offset 0h */
	for (i = 0; i < size; i++) {
		if (MEM32_BASE[i] != 0xffff0000) {
			return -13;
		} else {
			MEM16_BASE[i * 2] = 0xffff;
		}
	}

	/*  Read Check  */
	for (i = 0; i < size; i++) {
		if (MEM32_BASE[i] != 0xffffffff) {
			return -14;
		}
	}

	/*
	 * Additional verification
	 */

	/*  stage 1 => write 0  */
	for (i = 0; i < size; i++) {
		MEM_BASE[i] = PATTERN1;
	}

	/*  stage 2 => read 0, write 0xF  */
	for (i = 0; i < size; i++) {
		value = MEM_BASE[i];

		if (value != PATTERN1) {
			return -15;
		}
		MEM_BASE[i] = PATTERN2;
	}

	/*  stage 3 => read 0xF, write 0  */
	for (i = 0; i < size; i++) {
		value = MEM_BASE[i];
		if (value != PATTERN2) {
			return -16;
		}
		MEM_BASE[i] = PATTERN1;
	}

	/*  stage 4 => read 0, write 0xF  */
	for (i = 0; i < size; i++) {
		value = MEM_BASE[i];
		if (value != PATTERN1) {
			return -17;
		}
		MEM_BASE[i] = PATTERN2;
	}

	/*  stage 5 => read 0xF, write 0  */
	for (i = 0; i < size; i++) {
		value = MEM_BASE[i];
		if (value != PATTERN2) {
			return -18;
		}
		MEM_BASE[i] = PATTERN1;
	}

	/*  stage 6 => read 0  */
	for (i = 0; i < size; i++) {
		value = MEM_BASE[i];
		if (value != PATTERN1) {
			return -19;
		}
	}

	/*  1/2/4-byte combination test  */
	i = (unsigned int)(uintptr_t)MEM_BASE;

	while (i < (unsigned int)(uintptr_t)MEM_BASE + (size << 2)) {
		*((unsigned char *)(uintptr_t)i) = 0x78;
		i += 1;
		*((unsigned char *)(uintptr_t)i) = 0x56;
		i += 1;
		*((unsigned short *)(uintptr_t)i) = 0x1234;
		i += 2;
		*((unsigned int *)(uintptr_t)i) = 0x12345678;
		i += 4;
		*((unsigned short *)(uintptr_t)i) = 0x5678;
		i += 2;
		*((unsigned char *)(uintptr_t)i) = 0x34;
		i += 1;
		*((unsigned char *)(uintptr_t)i) = 0x12;
		i += 1;
		*((unsigned int *)(uintptr_t)i) = 0x12345678;
		i += 4;
		*((unsigned char *)(uintptr_t)i) = 0x78;
		i += 1;
		*((unsigned char *)(uintptr_t)i) = 0x56;
		i += 1;
		*((unsigned short *)(uintptr_t)i) = 0x1234;
		i += 2;
		*((unsigned int *)(uintptr_t)i) = 0x12345678;
		i += 4;
		*((unsigned short *)(uintptr_t)i) = 0x5678;
		i += 2;
		*((unsigned char *)(uintptr_t)i) = 0x34;
		i += 1;
		*((unsigned char *)(uintptr_t)i) = 0x12;
		i += 1;
		*((unsigned int *)(uintptr_t)i) = 0x12345678;
		i += 4;
	}
	for (i = 0; i < size; i++) {
		value = MEM_BASE[i];
		if (value != 0x12345678) {
			return -20;
		}
	}

	/*  Verify pattern 1 (0x00~0xff)  */
	pattern8 = 0x00;
	MEM8_BASE[0] = pattern8;
	for (i = 0; i < size * 4; i++) {
		unsigned char waddr8, raddr8;

		waddr8 = i + 1;
		raddr8 = i;
		if (i < size * 4 - 1)
			MEM8_BASE[waddr8] = pattern8 + 1;
		if (MEM8_BASE[raddr8] != pattern8) {
			return -21;
		}
		pattern8++;
	}

	/*  Verify pattern 2 (0x00~0xffff)  */
	pattern16 = 0x00;
	MEM16_BASE[0] = pattern16;
	for (i = 0; i < size * 2; i++) {
		if (i < size * 2 - 1)
			MEM16_BASE[i + 1] = pattern16 + 1;
		if (MEM16_BASE[i] != pattern16) {
			return -22;
		}
		pattern16++;
	}

	/*  Verify pattern 3 (0x00~0xffffffff)  */
	pattern32 = 0x00;
	MEM32_BASE[0] = pattern32;
	for (i = 0; i < size; i++) {
		if (i < size - 1)
			MEM32_BASE[i + 1] = pattern32 + 1;
		if (MEM32_BASE[i] != pattern32) {
			return -23;
		}
		pattern32++;
	}

	return 0;
}

#endif

/* ==================================================== */
#ifdef DRAMC_DEBUG_FULL_MEMORY_SCAN
enum {
	PATTERN1 = 0x5A5A5A5A,
	PATTERN2 = 0xA5A5A5A5
};

static inline uint32_t _errorExit ( uint32_t errcode) {
			return (-errcode);
}

static unsigned int u4Process = 0x00;


void mem_test_show_process(unsigned int index, unsigned int len)
{
	unsigned int u4NewProcess, u4OldProcess;

	u4NewProcess = (unsigned long long)index*100/(unsigned long long)len;

	if (index == 0x00) printk(BIOS_DEBUG, "0%%");
	if (u4NewProcess != u4Process) {

		u4Process = u4NewProcess;
		if ((u4Process%10) == 0x00) {
			printk(BIOS_DEBUG, "\n%d%%", u4Process);
		} else {
			printk(BIOS_DEBUG, ". ");
		}
	}

	if ((index != 0x00) && (index == (unsigned int)(len - 1))) {
		u4Process = 0x00;
		printk(BIOS_DEBUG, "\n100%%\n");
	}
}

int complex_mem_test1(unsigned int start, unsigned int len)
{
	volatile unsigned char *MEM8_BASE = (volatile unsigned char *) start;
	volatile unsigned short *MEM16_BASE = (volatile unsigned short *)start;
	volatile unsigned int *MEM32_BASE = (volatile unsigned int *)start;
	volatile unsigned int *MEM_BASE = (volatile unsigned int *)start;
	unsigned char pattern8;
	unsigned short pattern16;
	unsigned long long pattern64;
	unsigned int i, j, size, pattern32;
	unsigned int value, temp;
	volatile unsigned long long *MEM64_BASE;

	MEM64_BASE = (volatile unsigned long long *)start;

	size = len >> 2;

	*(volatile unsigned int *)0x10007000 = 0x22000000;
	printk(BIOS_DEBUG, "memory test start address = 0x%x, "
				"test length = 0x%x\n", start, len);

	/* === Verify the tied bits (tied low) === */
	for (i = 0; i < size; i++) {
		MEM32_BASE[i] = 0;
	}

	for (i = 0; i < size; i++) {
		mem_test_show_process(i, size);
		if (MEM32_BASE[i] != 0) {
			printk(BIOS_DEBUG, "Tied Low Test: Address %x "
						"not all zero, %x!\n\r",
					&MEM32_BASE[i], MEM32_BASE[i]);
			printk(BIOS_DEBUG, "....32bits all zero test: "
							"Fail!\n\r");
			_errorExit(1);
		} else {
			MEM32_BASE[i] = 0xffffffff;
		}
	}
	if (i == size)
	      printk(BIOS_DEBUG, "..32bits all zero test: Pass!\n\r");

	/* === Verify the tied bits (tied high) === */
	for (i = 0; i < size; i++) {
		mem_test_show_process(i, size);
		temp = MEM32_BASE[i];
		if (temp != 0xffffffff) {
			printk(BIOS_DEBUG, "Tied High Test: Address %x "
						"not equal 0xFFFFFFFF, "
					"%x!\n\r", &MEM32_BASE[i], temp);

			printk(BIOS_DEBUG, "....32bits all one test: "
							"Fail!\n\r");
			_errorExit(2);
		} else {
			MEM32_BASE[i] = 0x00;
		}
	}
	if (i == size)
	      printk(BIOS_DEBUG, "..32bits all one test: Pass!\n\r");

	/* === Verify pattern 1 (0x00~0xff) === */
	pattern8 = 0x00;
	for (i = 0; i < len; i++)
		MEM8_BASE[i] = pattern8++;

	pattern8 = 0x00;
	for (i = 0; i < len; i++) {
		mem_test_show_process(i, len);
		if (MEM8_BASE[i] != pattern8++) {
			printk(BIOS_DEBUG, "Address %x = %x, "
					"%x is expected!\n\r",
					&MEM8_BASE[i], MEM8_BASE[i], --pattern8);
			printk(BIOS_DEBUG, "....8bits 0x00~0xff pattern test: "
								"Fail!\n\r");
			_errorExit(3);
		}
	}
	if (i == len)
		printk(BIOS_DEBUG, "..8bits 0x00~0xff pattern test: Pass!\n\r");

	/* === Verify pattern 3 (0x0000, 0x0001, 0x0002, ... 0xFFFF) === */
	pattern16 = 0x00;
	for (i = 0; i < (len >> 1); i++)
		MEM16_BASE[i] = pattern16++;

	pattern16 = 0x00;
	for (i = 0; i < (len >> 1); i++) {
		mem_test_show_process(i, (len >> 1));
		temp = MEM16_BASE[i];
		if (temp != pattern16++) {

			printk(BIOS_DEBUG, "Address %x = %x, "
						"%x is expected!\n\r",
					&MEM16_BASE[i], temp, --pattern16);

			printk(BIOS_DEBUG, "....16bits 0x00~0xffff pattern test: "
								"Fail!\n\r");
			_errorExit(4);
		}
	}
	if (i == (len >> 1))
		printk(BIOS_DEBUG, "..16bits 0x00~0xffff pattern test: Pass!\n\r");

	/* === Verify pattern */
	/* 4 (0x00000000, 0x00000001, 0x00000002, ... 0xFFFFFFFF) === */
	pattern32 = 0x00;
	for (i = 0; i < (len >> 2); i++)
		MEM32_BASE[i] = pattern32++;
	pattern32 = 0x00;
	for (i = 0; i < (len >> 2); i++) {
		mem_test_show_process(i, size);
		if (MEM32_BASE[i] != pattern32++) {
			printk(BIOS_DEBUG, "Address %x = %x, %x is expected!\n\r",
					&MEM32_BASE[i], MEM32_BASE[i], --pattern32);
			printk(BIOS_DEBUG, "....32bits 0x00~0xffffffff pattern test: "
								"Fail!\n\r");
			_errorExit(5);
		}
	}
	if (i == (len >> 2))
		printk(BIOS_DEBUG, "..32bits 0x00~0xffffffff pattern test: Pass!\n\r");

	/* === Pattern 5: Filling memory range with 0xa5a5a5a5 === */
	for (i = 0; i < size; i++)
		MEM32_BASE[i] = 0xa5a5a5a5;

	/* === Read Check then Fill Memory with 00 Byte Pattern at offset 0h === */
	for (i = 0; i < size; i++) {
		mem_test_show_process(i, size);
		if (MEM32_BASE[i] != 0xa5a5a5a5) {
			printk(BIOS_DEBUG, "Address %x = %x, "
					"0xa5a5a5a5 is expected!\n\r",
					&MEM32_BASE[i], MEM32_BASE[i]);
			printk(BIOS_DEBUG, "....0xa5a5a5a5 pattern test: "
							"Fail!\n\r");
			_errorExit(6);
		} else {
			MEM8_BASE[i * 4] = 0x00;
		}
	}
	if (i == size)
		printk(BIOS_DEBUG, "..0xa5a5a5a5 pattern test: Pass!\n\r");

	/* === Read Check then Fill Memory with 00 Byte Pattern at offset 2h === */
	for (i = 0; i < size; i++) {
		mem_test_show_process(i, size);
		if (MEM32_BASE[i] != 0xa5a5a500) {
			printk(BIOS_DEBUG, "Address %x = %x, "
					"0xa5a5a500 is expected!\n\r",
					&MEM32_BASE[i], MEM32_BASE[i]);
			printk(BIOS_DEBUG, "....0xa5a5a500 pattern test: "
							"Fail!\n\r");
			_errorExit(7);
		} else {
			MEM8_BASE[i * 4 + 2] = 0x00;
		}
	}
	if (i == size)
		printk(BIOS_DEBUG, "..0xa5a5a500 pattern test: Pass!\n\r");

	/* === Read Check then Fill Memory with 00 Byte Pattern at offset 1h === */
	for (i = 0; i < size; i++) {
		mem_test_show_process(i, size);
		if (MEM32_BASE[i] != 0xa500a500) {
			printk(BIOS_DEBUG, "Address %x = %x, "
					"0xa500a500 is expected!\n\r",
					&MEM32_BASE[i], MEM32_BASE[i]);
			printk(BIOS_DEBUG, "....0xa500a500 pattern test: "
						"Fail!\n\r");
			_errorExit(8);
		} else {
			MEM8_BASE[i * 4 + 1] = 0x00;
		}
	}
	if (i == size)
		printk(BIOS_DEBUG, "..0xa500a500 pattern test: Pass!\n\r");

	/* === Read Check then Fill Memory with 00 Byte Pattern at offset 3h === */
	for (i = 0; i < size; i++) {
		mem_test_show_process(i, size);
		if (MEM32_BASE[i] != 0xa5000000) {
			printk(BIOS_DEBUG, "Address %x = %x, "
					"0xa5000000 is expected!\n\r",
					&MEM32_BASE[i], MEM32_BASE[i]);
			printk(BIOS_DEBUG, "....0xa5000000 pattern test: "
							"Fail!\n\r");
			_errorExit(9);
		} else {
			MEM8_BASE[i * 4 + 3] = 0x00;
		}
	}
	if (i == size)
		printk(BIOS_DEBUG, "..0xa5000000 pattern test: Pass!\n\r");

	/* === Read Check then Fill Memory with ffff Word Pattern at offset 1h === */
	for (i = 0; i < size; i++) {
		mem_test_show_process(i, size);
		if (MEM32_BASE[i] != 0x00000000) {
			printk(BIOS_DEBUG, "Address %x = %x, "
					"0x00000000 is expected!\n\r",
					&MEM32_BASE[i], MEM32_BASE[i]);
			printk(BIOS_DEBUG, "....0x00000000 pattern test: "
							"Fail!\n\r");
			_errorExit(10);
		}
	}
	if (i == size)
		printk(BIOS_DEBUG, "..0x00000000 pattern test: Pass!\n\r");

	/*
	 * Additional verification
	 */

	/* === stage 1 => write 0 === */
	for (i = 0; i < size; i++) {
		MEM_BASE[i] = PATTERN1;
	}

	/* === stage 2 => read 0, write 0xF === */
	for (i = 0; i < size; i++) {
		mem_test_show_process(i, size);
		value = MEM_BASE[i];
		if (value != PATTERN1) {
			printk(BIOS_DEBUG, "\nStage 2 error. Addr = %x, "
					"value = %x\n", &(MEM_BASE[i]), value);
			_errorExit(11);
		}
		MEM_BASE[i] = PATTERN2;
	}

	/* === stage 3 => read F === */
	for (i = 0; i < size; i++) {
		mem_test_show_process(i, size);
		value = MEM_BASE[i];
		if (value != PATTERN2) {
			printk(BIOS_DEBUG, "\nStage 3 error. Addr = %x, "
					"value = %x\n", &(MEM_BASE[i]), value);
			_errorExit(12);
		}
	}

	printk(BIOS_DEBUG, "..%x and %x Interleaving test: "
				"Pass!\n\r", PATTERN1, PATTERN2);
	return 0;
}
#endif
/* ==================================================== */

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
#include <arch/lib_helpers.h>	/* for raw_write_rmr_current() */
#include <arch/cache.h>
#include <smp/spinlock.h>	/* for spinlock */
#include <console/console.h>
#include <assert.h>
#include <soc/addressmap.h>
#include <soc/cpu.h>
#include <soc/mtcmos.h>

static uint32_t cpu_wakeup_bits; /* one bit per cpu */

static void set_arm64_reset_vector(unsigned int cpu_id, uintptr_t entry_64)
{
	uint32_t rv_base;

	switch (cpu_id) {
	case CPU_ID_LITTLE_CPU0:
	case CPU_ID_LITTLE_CPU1:
		rv_base = MTK_MCUCFG_BASE + 0x38 + (cpu_id * 8);
		break;
	case CPU_ID_BIG_CPU0:
	case CPU_ID_BIG_CPU1:
		rv_base = MTK_MCUCFG_BASE + 0x2a0 + (cpu_id * 8);
		break;
	default:
		/* should not come to here */
		return;
	}

	write32((void *)(uintptr_t)rv_base, (uint32_t)entry_64);
	write32((void *)(uintptr_t)(rv_base + 4), (uint32_t)(entry_64 >> 32));
}

static int remap_cpu_id(unsigned int cpu)
{
	enum {
		MAX_CPUS_PER_CLUSTER = 4,
		BIG_CPU0_LOGICAL_ID = 2
	};

	assert(cpu < CONFIG_MAX_CPUS);

	if (cpu >= BIG_CPU0_LOGICAL_ID)
		return MAX_CPUS_PER_CLUSTER - BIG_CPU0_LOGICAL_ID + cpu;
	return cpu;
}

DECLARE_SPIN_LOCK(mt8173_cpu_spinlock);

static inline void mt8173_cpu_lock(void)
{
	spin_lock(&mt8173_cpu_spinlock);
}

static inline void mt8173_cpu_unlock(void)
{
	spin_unlock(&mt8173_cpu_spinlock);
}

static inline uint64_t raw_read_cpuectlr_el1(void)
{
	uint64_t ectlr;

	asm volatile("mrs %0, s3_1_c15_c2_1\n\t" : "=r" (ectlr) : : "memory");
	return ectlr;
}

static inline void raw_write_cpuectlr_el1(uint64_t ectlr)
{
	asm volatile("msr s3_1_c15_c2_1, %0\n\t" : : "r" (ectlr) : "memory");
}

static inline void disable_interrupt(void)
{
	uint32_t val;

	enum {
		ENABLE_GRP0	 = 1 << 0,
		ENABLE_GRP1	 = 1 << 1,

		IRQ_BYP_DIS_GRP1 = 1 << 8,
		FIQ_BYP_DIS_GRP1 = 1 << 7,
		IRQ_BYP_DIS_GRP0 = 1 << 6,
		FIQ_BYP_DIS_GRP0 = 1 << 5,
	};

        /* disable secure, non-secure interrupts and disable their bypass */
	val = read32((void *)MTK_GICC_BASE); /* gicc ctlr */
	val &= ~(ENABLE_GRP0 | ENABLE_GRP1);
	val |= FIQ_BYP_DIS_GRP0 | FIQ_BYP_DIS_GRP1;
	val |= IRQ_BYP_DIS_GRP0 | IRQ_BYP_DIS_GRP1;
	write32((void *)MTK_GICC_BASE, val);
}

static void disable_smp(void)
{
	uint64_t ectlr;
	uint32_t midr_pn;

	enum {
		CPUECTLR_SMP_BIT = 1 << 6,

		MIDR_PN_SHIFT	 = 4,
		MIDR_PN_MASK	 = 0xfff,
		MIDR_PN_A57	 = 0xd07,
	};

	/* disable BIG L2 prefetches */
	midr_pn = (raw_read_midr_el1() >> MIDR_PN_SHIFT) & MIDR_PN_MASK;
	if (MIDR_PN_A57 == midr_pn) {
		ectlr = raw_read_cpuectlr_el1();
		ectlr |= (uint64_t)0x40 << 32;
		ectlr &= ~((uint64_t)0x1b << 32);
		raw_write_cpuectlr_el1(ectlr);
		isb();
		dsb();
	}

	/* take this cpu out of intra-cluster coherency */
	ectlr = raw_read_cpuectlr_el1();
	ectlr &= ~CPUECTLR_SMP_BIT;
	raw_write_cpuectlr_el1(ectlr);
	isb();
	dsb();
}

void cpu_sleep(unsigned int cpu, uintptr_t entry_64)
{
	unsigned int cpu_id;

	cpu_id = remap_cpu_id(cpu);

	do {
		wfe();

		mt8173_cpu_lock();
		if (cpu_wakeup_bits & (1 << cpu_id)) {
			cpu_wakeup_bits &= ~(1 << cpu_id);
			mt8173_cpu_unlock();
			break;
		}
		mt8173_cpu_unlock();
		isb();
		dsb();
	} while (1);

	printk(BIOS_DEBUG, "CPU%d(map to %d) wakeup, prepare reset to %p\n",
		cpu, cpu_id, (void *)entry_64);

	set_arm64_reset_vector(cpu_id, entry_64);

	cache_sync_instructions();

	disable_smp();
	disable_interrupt();

	raw_write_rmr_current(0x3); /* reset core: AARCH64 & RESET */
	isb();
	dsb();
	wfi();	/* trigger rmr */
	/* never return */
}

void cpu_wakeup(unsigned int cpu)
{
	unsigned int cpu_id;

	cpu_id = remap_cpu_id(cpu);

	mt8173_cpu_lock();
	cpu_wakeup_bits |= (1 << cpu_id);
	mt8173_cpu_unlock();

	sev();

	printk(BIOS_DEBUG, "Trying to wakeup CPU%d (map to CPU%d)\n",
		cpu, cpu_id);
}

/*
 * Following functions are needed only in romstage or ramstage. Secmon is built
 * as a rmodule, and is linked without --gc-sections. Use __SECMON__ for
 * conditional compilation to reduce secmon binary size.
 */
#if !defined(__SECMON__)
void start_cpu(unsigned int cpu, uintptr_t entry_64)
{
	unsigned int cpu_id;

	cpu_id = remap_cpu_id(cpu);

	printk(BIOS_DEBUG, "Starting CPU%d (map to CPU%d) @ %p\n",
		cpu, cpu_id, (void *)entry_64);

	set_arm64_reset_vector(cpu_id, entry_64);
	mtcmos_ctrl_cpu(cpu_id, STA_POWER_ON, 1);
}

void set_secondary_cpu_boot_arm64(void)
{
	uint32_t val;

	enum {
		LITTLE_CPUCFG = MTK_MCUCFG_BASE + 0x3c,
		BIG_CPUCFG = MTK_MCUCFG_BASE + 0x208
	};

	/* set secondary cpus: BIG arm64 boot mode */
	val = read32((void *)BIG_CPUCFG) | 0xf0000000;
	write32((void *)BIG_CPUCFG, val);

	/* set secondary cpus: LITTLE arm64 boot mode */
	val = read32((void *)LITTLE_CPUCFG) | 0x0000e000;
	write32((void *)LITTLE_CPUCFG, val);
}
#endif /* !defined(__SECMON__) */

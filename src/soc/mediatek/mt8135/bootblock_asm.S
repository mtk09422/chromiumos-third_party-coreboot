/*
 * This file is part of the coreboot project.
 *
 * Copyright 2014 MediaTek Inc.
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

#include <arch/asm.h>

.equ MODE_USR       ,0x10
.equ MODE_FIQ       ,0x11
.equ MODE_IRQ       ,0x12
.equ MODE_SVC       ,0x13
.equ MODE_MON       ,0x16
.equ MODE_ABT       ,0x17
.equ MODE_UNDEF     ,0x1B
.equ MODE_SYS       ,0x1F
.equ I_BIT          ,0x80
.equ F_BIT          ,0x40
.equ INT_BIT        ,0xC0

/* .extern enable_ca15 */
/* .extern slave_core */

.arm
ENTRY(_start)
	/*
	 * Set the cpu to Supervisor mode. Prefetch/Data aborts may happen early
	 * and crash before the abort handlers are installed, but at least the
	 * problem will show up near the code that causes it.
	 */
	msr	cpsr_cxf, #0xd3
	blx	_thumb_start
ENDPROC(_start)

.thumb
ENTRY(_thumb_start)
	/*
	 * enable I+Z+SMP bits and disable C bit
	 */
	mrc	p15, 0, r1, c1, c0, 0
	orr	r1, r1, #0x1840
	bic	r1, r1, #0x4
	mcr	p15, 0, r1, c1, c0, 0

	/*
	 * Enable DVM in ACTLR (no effects for CA15)
	 */
	mrc	p15, 0, r1, c1, c0, 1
	bic	r1, #0x8000
	mcr	p15, 0, r1, c1, c0, 1

	bl	arm_init_caches

	/*
	 * From Cortex-A Series Programmer's Guide:
	 * Only CPU 0 performs initialization. Other CPUs go into WFI
	 * to do this, first work out which CPU this is
	 * this code typically is run before any other initialization step
	 */
	mrc p15, 0, r1, c0, c0, 5	@ Read Multiprocessor Affinity Register
	orr r1, r1, r1, lsr #7
	and r1, r1, #0x3 @ Extract CPU ID bits
	cmp r1, #0
	bne wait_for_interrupt		@ If this is not core0, wait

	/*
	 * Initialize the stack to a known value. This is used to check for
	 * stack overflow later in the boot process.
	 */
	ldr	r0, =_stack
	ldr	r1, =_estack
	ldr	r2, =0xdeadbeef
init_stack_loop:
	str	r2, [r0]
	add	r0, #4
	cmp	r0, r1
	bne	init_stack_loop

/* Set stackpointer in internal RAM to call bootblock main() */
call_bootblock:
	ldr	sp, =_estack /* Set up stack pointer */
	ldr	r0,=0x00000000
	 /*
	  * The current design of cpu_info places the
	  * struct at the top of the stack. The number of
	  * words pushed must be at least as large as that
	  * struct.
	  */
	sub	sp, sp, #16
	/*
	 * Use "bl" instead of "b" even though we do not intend to return.
	 * "bl" gets compiled to "blx" if we're transitioning from ARM to
	 * Thumb. However, "b" will not and GCC may attempt to create a
	 * wrapper which is currently broken.
	 */
	bl	main

.equ	GIC_CPU_BASE ,0x10212000
wait_for_interrupt:
	movw	r2, #:lower16:GIC_CPU_BASE
	movt	r2, #:upper16:GIC_CPU_BASE
	mov	r0, #0xF0
	str	r0, [r2, #4]
	mov	r0, #1
	str	r0, [r2, #0]

	adr	r0, 2f
	ldr	r0, [r0, r1, lsl #2]
	movw	r1, #:lower16:0x800023f8
	movt	r1, #:upper16:0x800023f8

1:	wfi

	ldm	r1, {r2, lr}
	cmp	r0, r2
	bne	1b

	bx	lr			@ back to my caller

.align 2
2:	.word	0x534c4131
	.word	0x534c4131
	.word	0x4c415332
	.word	0x41534c33

ENDPROC(_thumb_start)

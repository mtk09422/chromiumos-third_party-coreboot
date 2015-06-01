/*
 * This file is part of the coreboot project.
 *
 * Copyright 2014 Google Inc.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA, 02110-1301 USA
 */

#include <arch/cpu.h>
#include <arch/io.h>
#include <arch/psci.h>
#include <soc/cpu.h>

static uintptr_t cpu_on_entry_point;

void psci_soc_init(uintptr_t cpu_on_entry)
{
	cpu_on_entry_point = cpu_on_entry;
}

static size_t children_at_level(int parent_level, uint64_t mpidr)
{
	if (mpidr != 0)
		return 0;

	switch (parent_level) {
	case PSCI_AFFINITY_ROOT:
		return 1;
	case PSCI_AFFINITY_LEVEL_3:
		return 1;
	case PSCI_AFFINITY_LEVEL_2:
		return 1;
	case PSCI_AFFINITY_LEVEL_1:
		return 2;
	case PSCI_AFFINITY_LEVEL_0:
		return 0;
	default:
		return 0;
	}
}

static int cmd_prepare(struct psci_cmd *cmd)
{
	int ret;

	switch (cmd->type) {
	case PSCI_CMD_ON:
		ret = PSCI_RET_SUCCESS;
		break;
	case PSCI_CMD_OFF:
		ret = PSCI_RET_SUCCESS;
		break;
	default:
		ret = PSCI_RET_NOT_SUPPORTED;
		break;
	}
	return ret;
}

static int cmd_commit(struct psci_cmd *cmd)
{
	int ret;
	unsigned int cpu;

	cpu = cmd->target->cpu_state.ci->id;

	switch (cmd->type) {
	case PSCI_CMD_ON:
		cpu_wakeup(cpu);
		ret = PSCI_RET_SUCCESS;
		break;
	case PSCI_CMD_OFF:
		cpu_sleep(cpu, cpu_on_entry_point);
		ret = PSCI_RET_SUCCESS;
		break;
	default:
		ret = PSCI_RET_NOT_SUPPORTED;
		break;
	}
	return ret;
}

struct psci_soc_ops soc_psci_ops = {
	.children_at_level = &children_at_level,
	.cmd_prepare = &cmd_prepare,
	.cmd_commit = &cmd_commit,
};

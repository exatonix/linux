/*
 * PCIe host controller common functions for HiSilicon SoCs
 *
 * Copyright (C) 2015 HiSilicon Co., Ltd. http://www.hisilicon.com
 *
 * Author: Zhou Wang <wangzhou1@hisilicon.com>
 *         Dacai Zhu <zhudacai@hisilicon.com>
 *         Gabriele Paoloni <gabriele.paoloni@huawei.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/acpi.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/mfd/syscon.h>
#include <linux/of_pci.h>
#include <linux/regmap.h>

#include "pcie-designware.h"
#include "pcie-hisi.h"

#ifdef CONFIG_PCI_HISI_ACPI

/* Hip05 support ECAM to access EP configuration space but not support RC. */
static int hisi_rd_other_conf(struct pcie_port *pp, struct pci_bus *bus,
		unsigned int devfn, int where, int size, u32 *value)
{
	void __iomem *addr = pp->dbi_base;
	int ret;

	if (bus->parent->number == pp->root_bus_nr)
		writel(PCIE_ATU_TYPE_CFG0, addr + PCIE_ATU_CR1);
	else
		writel(PCIE_ATU_TYPE_CFG1, addr + PCIE_ATU_CR1);
	ret = pci_generic_config_read(bus, devfn, where, size, value);

	return ret;
}

static int hisi_wr_other_conf(struct pcie_port *pp, struct pci_bus *bus,
		unsigned int devfn, int where, int size, u32 value)
{
	void __iomem *addr = pp->dbi_base;
	int ret;

	if (bus->parent->number == pp->root_bus_nr)
		writel(PCIE_ATU_TYPE_CFG0, addr + PCIE_ATU_CR1);
	else
		writel(PCIE_ATU_TYPE_CFG1, addr + PCIE_ATU_CR1);
	ret = pci_generic_config_write(bus, devfn, where, size, value);

	return ret;
}

#endif /* CONFIG_PCI_HISI_ACPI */

/* HipXX PCIe host only supports 32-bit config access */
static int hisi_pcie_cfg_read(struct pcie_port *pp, int where, int size,
			      u32 *val)
{
	u32 reg;
	u32 reg_val;
	struct hisi_pcie *pcie = to_hisi_pcie(pp);
	void *walker = &reg_val;

	walker += (where & 0x3);
	reg = where & ~0x3;
	reg_val = hisi_pcie_apb_readl(pcie, reg);

	if (size == 1)
		*val = *(u8 __force *) walker;
	else if (size == 2)
		*val = *(u16 __force *) walker;
	else if (size == 4)
		*val = reg_val;
	else
		return PCIBIOS_BAD_REGISTER_NUMBER;

	return PCIBIOS_SUCCESSFUL;
}

/* HipXX PCIe host only supports 32-bit config access */
static int hisi_pcie_cfg_write(struct pcie_port *pp, int where, int  size,
				u32 val)
{
	u32 reg_val;
	u32 reg;
	struct hisi_pcie *pcie = to_hisi_pcie(pp);
	void *walker = &reg_val;

	walker += (where & 0x3);
	reg = where & ~0x3;
	if (size == 4)
		hisi_pcie_apb_writel(pcie, val, reg);
	else if (size == 2) {
		reg_val = hisi_pcie_apb_readl(pcie, reg);
		*(u16 __force *) walker = val;
		hisi_pcie_apb_writel(pcie, reg_val, reg);
	} else if (size == 1) {
		reg_val = hisi_pcie_apb_readl(pcie, reg);
		*(u8 __force *) walker = val;
		hisi_pcie_apb_writel(pcie, reg_val, reg);
	} else
		return PCIBIOS_BAD_REGISTER_NUMBER;

	return PCIBIOS_SUCCESSFUL;
}

static int hisi_pcie_link_up(struct pcie_port *pp)
{
	struct hisi_pcie *hisi_pcie = to_hisi_pcie(pp);

	return hisi_pcie->soc_ops->hisi_pcie_link_up(hisi_pcie);
}

struct pcie_host_ops hisi_pcie_host_ops = {
#ifdef CONFIG_PCI_HISI_ACPI
	.rd_other_conf = hisi_rd_other_conf,
	.wr_other_conf = hisi_wr_other_conf,
#endif /* CONFIG_ACPI_HOST_GENERIC */
	.rd_own_conf = hisi_pcie_cfg_read,
	.wr_own_conf = hisi_pcie_cfg_write,
	.link_up = hisi_pcie_link_up,
};

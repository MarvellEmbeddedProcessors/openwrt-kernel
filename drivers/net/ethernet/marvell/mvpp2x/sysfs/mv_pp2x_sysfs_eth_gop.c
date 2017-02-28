/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.


********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the General
Public License Version 2, June 1991 (the "GPL License"), a copy of which is
available along with the File in the license.txt file or by writing to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
DISCLAIMED.  The GPL License provides additional details about this warranty
disclaimer.
*******************************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/capability.h>
#include <linux/platform_device.h>
#include <linux/netdevice.h>

#include "mv_pp2x_sysfs.h"

static ssize_t mv_gop_help(char *b)
{
	int o = 0;

	o += sprintf(b+o, "cat               gopBases        - show GOP Unit Virtual Bases\n");
	o += sprintf(b+o, "cat               xpcsGlRegs      - show XPCS Global registers\n");
	o += sprintf(b+o, "echo [p]        > status_show     - show GOP Port status\n");
	o += sprintf(b+o, "echo [p]        > gmacRegs        - show GMAC registers for port <p>\n");
	o += sprintf(b+o, "echo [p]        > xlgmacRegs      - show XLG MAC registers for port <p>\n");
	o += sprintf(b+o, "echo [p]        > mibCntrs        - show MIB counters for port <p>\n");
	o += sprintf(b+o, "echo [p]        > xpcsLaneRegs    - show XPCS Lane registers for lane <p>\n");
	return o;
}

static ssize_t mv_gop_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	const char      *name = attr->attr.name;
	int             off = 0;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (!strcmp(name, "xpcsGlRegs"))
		mv_gop110_xpcs_gl_regs_dump(&sysfs_cur_hw->gop);
	else if (!strcmp(name, "gopBases"))
		mv_gop110_register_bases_dump(&sysfs_cur_hw->gop);
	else
		off = mv_gop_help(buf);

	return off;
}

static ssize_t mv_gop_port_store(struct device *dev,
				   struct device_attribute *attr, const char *buf, size_t len)
{
	const char      *name = attr->attr.name;
	int             err;
	unsigned int    p, v, a;
	unsigned long   flags;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	/* Read port and value */
	err = p = v = a = 0;
	sscanf(buf, "%d %d %d", &p, &v, &a);

	local_irq_save(flags);


	if (!strcmp(name, "gmacRegs")) {
		mv_gop110_gmac_regs_dump(&sysfs_cur_hw->gop, p);
	} else if (!strcmp(name, "gmacRead")) {
		mv_gop110_gmac_read(&sysfs_cur_hw->gop, p, v);
	} else if (!strcmp(name, "gmacWrite")) {
		mv_gop110_gmac_write(&sysfs_cur_hw->gop, p, v, a);
	} else if (!strcmp(name, "smiRead")) {
		mv_gop110_smi_read(&sysfs_cur_hw->gop, p);
	} else if (!strcmp(name, "smiWrite")) {
		mv_gop110_smi_write(&sysfs_cur_hw->gop, p, v);
	} else if (!strcmp(name, "xmibRead")) {
		mv_gop110_xmib_mac_read(&sysfs_cur_hw->gop, p, v);
	} else if (!strcmp(name, "xmibWrite")) {
		mv_gop110_xmib_mac_write(&sysfs_cur_hw->gop, p, v, a);
	} else if (!strcmp(name, "xlgRead")) {
		mv_gop110_xlg_mac_read(&sysfs_cur_hw->gop, p, v);
	} else if (!strcmp(name, "xlgWrite")) {
		mv_gop110_xlg_mac_write(&sysfs_cur_hw->gop, p, v, a);
	} else if (!strcmp(name, "xpcsLaneRead")) {
		mv_gop110_xpcs_lane_read(&sysfs_cur_hw->gop, p, v);
	} else if (!strcmp(name, "xpcsLaneWrite")) {
		mv_gop110_xpcs_lane_write(&sysfs_cur_hw->gop, p, v, a);
	} else if (!strcmp(name, "xpcsGlobalRead")) {
		mv_gop110_xpcs_global_read(&sysfs_cur_hw->gop, p);
	} else if (!strcmp(name, "xpcsGlobalWrite")) {
		mv_gop110_xpcs_global_write(&sysfs_cur_hw->gop, p, v);
	} else if (!strcmp(name, "xlgmacRegs")) {
		mv_gop110_xlg_mac_regs_dump(&sysfs_cur_hw->gop, p);
	} else if (!strcmp(name, "mibCntrs")) {
		mv_gop110_mib_counters_show(&sysfs_cur_hw->gop, p);
	} else if (!strcmp(name, "xpcsLaneRegs")) {
		mv_gop110_xpcs_lane_regs_dump(&sysfs_cur_hw->gop, p);
	} else if (!strcmp(name, "status_show")) {
		mv_gop110_status_show(&sysfs_cur_hw->gop, sysfs_cur_priv, p);
	} else {
		err = 1;
		printk(KERN_ERR "%s: illegal operation <%s>\n", __func__, attr->attr.name);
	}

	local_irq_restore(flags);

	if (err)
		printk(KERN_ERR "%s: error %d\n", __func__, err);

	return err ? -EINVAL : len;
}


static DEVICE_ATTR(help,          S_IRUSR, mv_gop_show, NULL);
static DEVICE_ATTR(xpcsGlRegs,	S_IRUSR, mv_gop_show, NULL);
static DEVICE_ATTR(gmacRegs,      S_IWUSR, NULL, mv_gop_port_store);
static DEVICE_ATTR(gmacRead,      S_IWUSR, NULL, mv_gop_port_store);
static DEVICE_ATTR(gmacWrite,      S_IWUSR, NULL, mv_gop_port_store);
static DEVICE_ATTR(smiRead,      S_IWUSR, NULL, mv_gop_port_store);
static DEVICE_ATTR(smiWrite,      S_IWUSR, NULL, mv_gop_port_store);
static DEVICE_ATTR(xmibRead,      S_IWUSR, NULL, mv_gop_port_store);
static DEVICE_ATTR(xmibWrite,      S_IWUSR, NULL, mv_gop_port_store);
static DEVICE_ATTR(xlgRead,      S_IWUSR, NULL, mv_gop_port_store);
static DEVICE_ATTR(xlgWrite,      S_IWUSR, NULL, mv_gop_port_store);
static DEVICE_ATTR(xpcsLaneRead,      S_IWUSR, NULL, mv_gop_port_store);
static DEVICE_ATTR(xpcsLaneWrite,      S_IWUSR, NULL, mv_gop_port_store);
static DEVICE_ATTR(xpcsGlobalRead,      S_IWUSR, NULL, mv_gop_port_store);
static DEVICE_ATTR(xpcsGlobalWrite,      S_IWUSR, NULL, mv_gop_port_store);
static DEVICE_ATTR(xlgmacRegs,    S_IWUSR, NULL, mv_gop_port_store);
static DEVICE_ATTR(mibCntrs,      S_IWUSR, NULL, mv_gop_port_store);
static DEVICE_ATTR(xpcsLaneRegs,  S_IWUSR, NULL, mv_gop_port_store);
static DEVICE_ATTR(status_show,    S_IWUSR, NULL, mv_gop_port_store);



static struct attribute *mv_gop_attrs[] = {
	&dev_attr_help.attr,
	&dev_attr_gmacRead.attr,
	&dev_attr_gmacWrite.attr,
	&dev_attr_smiRead.attr,
	&dev_attr_smiWrite.attr,
	&dev_attr_xmibRead.attr,
	&dev_attr_xmibWrite.attr,
	&dev_attr_xlgRead.attr,
	&dev_attr_xlgWrite.attr,
	&dev_attr_xpcsLaneRead.attr,
	&dev_attr_xpcsLaneWrite.attr,
	&dev_attr_xpcsGlobalRead.attr,
	&dev_attr_xpcsGlobalWrite.attr,
	&dev_attr_xpcsGlRegs.attr,
	&dev_attr_gmacRegs.attr,
	&dev_attr_xlgmacRegs.attr,
	&dev_attr_mibCntrs.attr,
	&dev_attr_xpcsLaneRegs.attr,
	&dev_attr_status_show.attr,
	NULL
};

static struct attribute_group mv_gop_group = {
	.name = "gop",
	.attrs = mv_gop_attrs,
};

int mv_gop_sysfs_init(struct kobject *gbe_kobj)
{
	int err;

	err = sysfs_create_group(gbe_kobj, &mv_gop_group);
	if (err)
		pr_err("sysfs group %s failed %d\n", mv_gop_group.name, err);

	return err;
}

int mv_gop_sysfs_exit(struct kobject *gbe_kobj)
{
	sysfs_remove_group(gbe_kobj, &mv_gop_group);

	return 0;
}

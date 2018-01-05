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

static ssize_t mv_fca_help(char *b)
{
	int o = 0;

	o += sprintf(b+o, "echo [p] [0|1]      > set_xoff_xon     - set GOP Port, 0 - XOFF, 1 - XON\n");
	o += sprintf(b+o, "echo [p] [timer]    > set_periodic	  - set periodic timer\n");

	return o;
}

static ssize_t mv_fca_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	const char      *name = attr->attr.name;
	int             off = 0;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (!strcmp(name, "set_xoff_xon"))
		mv_gop110_register_bases_dump(&sysfs_cur_hw->gop);
	else
		off = mv_fca_help(buf);

	return off;
}

static ssize_t mv_fca_port_store(struct device *dev,
				   struct device_attribute *attr, const char *buf, size_t len)
{
	const char      *name = attr->attr.name;
	int             err;
	u64    p, v, a;
	unsigned long   flags;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	/* Read port and value */
	err = p = v = a = 0;
	sscanf(buf, "%lld %lld %lld", &p, &v, &a);

	local_irq_save(flags);

	if (!strcmp(name, "set_xoff_xon")) {
		mv_gop110_netc_xon_set(&sysfs_cur_hw->gop, p, v);
	} else if (!strcmp(name, "set_periodic")) {
		mv_gop110_fca_set_periodic_timer(&sysfs_cur_hw->gop, p, v);
	} else {
		err = 1;
		printk(KERN_ERR "%s: illegal operation <%s>\n", __func__, attr->attr.name);
	}

	local_irq_restore(flags);

	if (err)
		printk(KERN_ERR "%s: error %d\n", __func__, err);

	return err ? -EINVAL : len;
}


static DEVICE_ATTR(help,          S_IRUSR, mv_fca_show, NULL);
static DEVICE_ATTR(set_xoff_xon,      S_IWUSR, NULL, mv_fca_port_store);
static DEVICE_ATTR(set_periodic,      S_IWUSR, NULL, mv_fca_port_store);

static struct attribute *mv_fca_attrs[] = {
	&dev_attr_help.attr,
	&dev_attr_set_xoff_xon.attr,
	&dev_attr_set_periodic.attr,
	NULL
};

static struct attribute_group mv_fca_group = {
	.name = "fca",
	.attrs = mv_fca_attrs,
};

int mv_fca_sysfs_init(struct kobject *gbe_kobj)
{
	int err;

	err = sysfs_create_group(gbe_kobj, &mv_fca_group);
	if (err)
		pr_err("sysfs group %s failed %d\n", mv_fca_group.name, err);

	return err;
}

int mv_fca_sysfs_exit(struct kobject *gbe_kobj)
{
	sysfs_remove_group(gbe_kobj, &mv_fca_group);

	return 0;
}

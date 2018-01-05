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
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include "mv_pp2x_sysfs.h"
#include "mv_pp2x_sysfs_prs.h"


static struct kobject *prs_kobj;

static ssize_t mv_prs_high_help(char *b)
{
	int o = 0;

	o += scnprintf(b + o, PAGE_SIZE - o, "cd                 debug       - move to parser low level sysfs directory\n");
	o += scnprintf(b + o, PAGE_SIZE - o, "cat                dump        - dump all valid HW entries\n");

	return o;
}


static ssize_t mv_prs_high_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	const char      *name = attr->attr.name;
	int             off = 0;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (!strcmp(name, "dump"))
		mv_pp2x_prs_hw_dump(sysfs_cur_hw);
	else
		off += mv_prs_high_help(buf);

	return off;
}



static DEVICE_ATTR(dump,		S_IRUSR, mv_prs_high_show, NULL);
static DEVICE_ATTR(help,		S_IRUSR, mv_prs_high_show, NULL);


static struct attribute *prs_high_attrs[] = {
	&dev_attr_dump.attr,
	&dev_attr_help.attr,
    NULL
};

static struct attribute_group prs_high_group = {
	.attrs = prs_high_attrs,
};

int mv_pp2_prs_high_sysfs_init(struct kobject *pp2_kobj)
{
	int err;

	prs_kobj = kobject_create_and_add("prs", pp2_kobj);

	if (!prs_kobj) {
		pr_err("%s: cannot create gbe kobject\n", __func__);
		return -ENOMEM;
	}

	err = sysfs_create_group(prs_kobj, &prs_high_group);
	if (err)
		pr_err("sysfs group failed %d\n", err);

	mv_pp2_prs_low_sysfs_init(prs_kobj);

	return err;

}

int mv_pp2_prs_high_sysfs_exit(struct kobject *pp2_kobj)
{
	mv_pp2_prs_low_sysfs_exit(prs_kobj);

	sysfs_remove_group(pp2_kobj, &prs_high_group);

	return 0;
}


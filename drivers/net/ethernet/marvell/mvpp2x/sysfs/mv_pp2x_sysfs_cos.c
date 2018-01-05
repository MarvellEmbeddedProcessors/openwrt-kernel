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

static ssize_t mv_cos_help(char *buf)
{
	int off = 0;
	off += scnprintf(buf + off, PAGE_SIZE,  "echo [if_name] [cos_mode] >  classifier_set  - Set the cos classifier on the interface\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo [if_name]            >  classifier_show  - Show the cos classifier on the interface\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "     [cos_mode]  - 0 - VLAN Pri\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "                 - 1 - DSCP Pri\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "                 - 2 - VLAN_DSCP Pri, tagged packet based on VLAN pri, Untagged IP based on DSCP\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "                 - 3 - DSCP_VLAN Pri, IP packet based on DSCP, tagged non-IP packet based on VLAN pri\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo [if_name] [map]     >  pri_map_set  - Set the cos mapping, map the cos value to rxq\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo [if_name]           >  pri_map_show - Show the cos mapping, map the cos value to rxq\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "     [map]       - The index of each nibble indicates a cos value, the value of nibble indicates the rxq\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo [if_name] [default] >  cos_default_set  - Set the default cos value for untagged or non-IP packet\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo [if_name]           >  cos_default_show - Get the default cos value for untagged or non-IP packet\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "\n");

	return off;
}

static ssize_t mv_cos_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int off = 0;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	off += mv_cos_help(buf);

	return off;
}

static ssize_t mv_cos_store(struct device *dev,
				   struct device_attribute *attr, const char *buf, size_t len)
{
	const char      *name = attr->attr.name;
	int             err;
	unsigned int    b;
	char		if_name[10];
	struct net_device *netdev;
	struct mv_pp2x_port *port;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	/* Read port and value */
	err = b = 0;

	sscanf(buf, "%s %x", if_name, &b);

	netdev = dev_get_by_name(&init_net, if_name);
	if (!netdev) {
		printk("%s: illegal interface <%s>\n", __func__, if_name);
		return -EINVAL;
	}
	port = netdev_priv(netdev);

	if (!strcmp(name, "classifier_set")) {
		mv_pp2x_cos_classifier_set(port, b);
	} else if (!strcmp(name, "classifier_show")) {
		printk("On port %s, classifier mode is %d\n", if_name, mv_pp2x_cos_classifier_get(port));
	} else if (!strcmp(name, "pri_map_set")) {
		mv_pp2x_cos_pri_map_set(port, b);
	} else if (!strcmp(name, "pri_map_show")) {
		printk("On port %s, pri_map is 0x%x\n", if_name, mv_pp2x_cos_pri_map_get(port));
	} else if (!strcmp(name, "cos_default_set")) {
		mv_pp2x_cos_default_value_set(port, b);
	} else if (!strcmp(name, "cos_default_show")) {
		printk("On port %s, default cos value is %d\n", if_name, mv_pp2x_cos_default_value_get(port));
	} else {
		err = 1;
		printk("%s: illegal operation <%s>\n", __func__, attr->attr.name);
	}

	if (err)
		printk("%s: error %d\n", __func__, err);
	dev_put(netdev);
	return err ? -EINVAL : len;
}

static DEVICE_ATTR(classifier_set,	S_IWUSR, NULL, mv_cos_store);
static DEVICE_ATTR(classifier_show,	S_IWUSR, NULL, mv_cos_store);
static DEVICE_ATTR(pri_map_set,		S_IWUSR, NULL, mv_cos_store);
static DEVICE_ATTR(pri_map_show,	S_IWUSR, NULL, mv_cos_store);
static DEVICE_ATTR(cos_default_set,	S_IWUSR, NULL, mv_cos_store);
static DEVICE_ATTR(cos_default_show,	S_IWUSR, NULL, mv_cos_store);
static DEVICE_ATTR(help,		S_IRUSR, mv_cos_show, NULL);

static struct attribute *cos_attrs[] = {
	&dev_attr_classifier_set.attr,
	&dev_attr_classifier_show.attr,
	&dev_attr_pri_map_set.attr,
	&dev_attr_pri_map_show.attr,
	&dev_attr_cos_default_set.attr,
	&dev_attr_cos_default_show.attr,
	&dev_attr_help.attr,
	NULL
};

static struct attribute_group cos_group = {
	.name = "cos",
	.attrs = cos_attrs,
};

int mv_pp2_cos_sysfs_init(struct kobject *pp2_kobj)
{
	int err = 0;

	err = sysfs_create_group(pp2_kobj, &cos_group);
	if (err)
		printk("sysfs group %s failed %d\n", cos_group.name, err);

	return err;
}

int mv_pp2_cos_sysfs_exit(struct kobject *pp2_kobj)
{
	sysfs_remove_group(pp2_kobj, &cos_group);

	return 0;
}


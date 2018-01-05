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
#include "mv_pp2x_debug.h"

void mv_pp2x_syfs_cpn_set(int index)
{
	struct device *pp2_dev;
	struct mv_pp2x *priv;

	pp2_dev = bus_find_device_by_name(&platform_bus_type, NULL, pp2_dev_name[index]);
	priv = dev_get_drvdata(pp2_dev);

	sysfs_cur_priv = priv;
	sysfs_cur_hw = &priv->hw;
}

static int mv_pp2x_parse_mac_address(char *buf, u32 *macaddr_parts)
{
	if (sscanf(buf, "%x:%x:%x:%x:%x:%x",
		   &macaddr_parts[0], &macaddr_parts[1],
		   &macaddr_parts[2], &macaddr_parts[3],
		   &macaddr_parts[4], &macaddr_parts[5]) == ETH_ALEN)
		return MV_OK;
	else
		return MV_ERROR;
}

static ssize_t mv_debug_help(char *buf)
{
	int off = 0;
	off += scnprintf(buf + off, PAGE_SIZE,  "echo [if_name] [cpu]     >  bind_cpu - Bind the interface to dedicated CPU\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "     NOTE: bind_cpu only valid when rss is disabled\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo offset val	        > mv_pp2x_reg_write    - Write mvpp2 register.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo offset		> mv_pp2x_reg_read     - Read mvpp2 register.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo offset val cpu	> mv_pp2x_reg_percpu_write    - Write mvpp2 percpu register.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo offset cpu	        > mv_pp2x_reg_percpu_read     - Read mvpp2 percpu register.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo val        > debug_param          - Set global debug_param.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "cat             debug_param           - Get global debug_param.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo val       > cpn_index            - Set cpn_index to use for sysfs commands.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo [if_name] [mac_addr]     > uc_filter_add - Add UC MAC to filter list on the device\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo [if_name] [mac_addr]     > uc_filter_del - Del UC MAC from filter list on the device\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo [if_name]                > uc_filter_flush - flush UC MAC from filter list on the device\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo [if_name]                > uc_filter_dump - Dump UC MAC in filter list on the device\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "     mac_addr format: ff:ff:ff:ff:ff:ff\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo [if_name] [register]  [value]   >  phy_reg_write - Write phy register\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo [if_name] [register]     	      >  phy_reg_read - Read phy register\n");


	return off;
}


static ssize_t mv_debug_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int off = 0;
	const char    *name = attr->attr.name;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (!strcmp(name, "debug_param")) {
		DBG_MSG("debug_param(%d)\n", mv_pp2x_debug_param_get());
	}
	else
		off += mv_debug_help(buf);

	return off;
}

static ssize_t mv_debug_store(struct device *dev,
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

	if (!strcmp(name, "bind_cpu")) {
		mv_pp2x_port_bind_cpu_set(port, b);
	} else {
		err = 1;
		printk("%s: illegal operation <%s>\n", __func__, attr->attr.name);
	}

	if (err)
		printk("%s: error %d\n", __func__, err);
	dev_put(netdev);
	return err ? -EINVAL : len;
}

static ssize_t mv_debug_store_unsigned(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t len)
{
	const char    *name = attr->attr.name;
	unsigned int  err = 0, a = 0, b = 0, c = 0, d = 0;
	unsigned long flags;
	u32 val;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	sscanf(buf, "%x %x %x %x", &a, &b, &c, &d);

	local_irq_save(flags);

	if (!strcmp(name, "mv_pp2x_reg_read")) {
		val = mv_pp2x_read(sysfs_cur_hw, a);
		printk("mv_pp2x_read(0x%x)=0x%x\n", a, val);
	} else if (!strcmp(name, "mv_pp2x_reg_write")) {
		mv_pp2x_write(sysfs_cur_hw, a, b);
		val = mv_pp2x_read(sysfs_cur_hw, a);
		printk("mv_pp2x_write_read(0x%x)=0x%x\n", a, val);
	} else if (!strcmp(name, "mv_pp2x_reg_percpu_read")) {
		val = mv_pp22_thread_relaxed_read(sysfs_cur_hw, b, a);
		printk("mv_pp2x_read(0x%x)=0x%x\n", a, val);
	} else if (!strcmp(name, "mv_pp2x_reg_percpu_write")) {
		mv_pp22_thread_relaxed_write(sysfs_cur_hw, c, a, b);
		val = mv_pp22_thread_relaxed_read(sysfs_cur_hw, c, a);
		printk("mv_pp2x_write_read(0x%x)=0x%x\n", a, val);
	} else if (!strcmp(name, "debug_param")) {
		mv_pp2x_debug_param_set(a);
	} else if (!strcmp(name, "cpn_index")) {
		mv_pp2x_syfs_cpn_set(a);
	}
	else {
		err = 1;
		printk(KERN_ERR "%s: illegal operation <%s>\n", __func__, attr->attr.name);
	}
	local_irq_restore(flags);

	if (err)
		printk(KERN_ERR "%s: <%s>, error %d\n", __func__, attr->attr.name, err);

	return err ? -EINVAL : len;
}

static ssize_t mv_debug_store_mac(struct device *dev,
				  struct device_attribute *attr, const char *buf, size_t len)
{
	const char      *name = attr->attr.name;
	int             err = 0;
	u32		b[ETH_ALEN];
	u8		mac_uc[ETH_ALEN];
	char		if_name[10];
	char		mac[30];
	struct net_device *netdev;
	int i;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	sscanf(buf, "%s %s", if_name, mac);

	netdev = dev_get_by_name(&init_net, if_name);
	if (!netdev) {
		printk("%s: illegal interface <%s>\n", __func__, if_name);
		return -EINVAL;
	}

	if (!strcmp(name, "uc_filter_add")) {
		if (mv_pp2x_parse_mac_address(mac, b)) {
			pr_err("%s: illegal mac address input\n", mac);
			err = 1;
			goto error;
		}
		for (i = 0; i < ETH_ALEN; i++)
			mac_uc[i] = (u8)b[i];
		dev_uc_add(netdev, mac_uc);
	} else if (!strcmp(name, "uc_filter_del")) {
		if (mv_pp2x_parse_mac_address(mac, b)) {
			pr_err("%s: illegal mac address input\n", mac);
			err = 1;
			goto error;
		}
		for (i = 0; i < ETH_ALEN; i++)
			mac_uc[i] = (u8)b[i];
		dev_uc_del(netdev, mac_uc);
	} else if (!strcmp(name, "uc_filter_flush")) {
		struct netdev_hw_addr *ha;

		netdev_for_each_uc_addr(ha, netdev)
			dev_uc_del(netdev, ha->addr);
	} else if (!strcmp(name, "uc_filter_dump")) {
		struct netdev_hw_addr *ha;

		if (netdev_uc_count(netdev) == 0) {
				pr_info("%s: UC list is empty\n", if_name);
		} else {
			pr_info("%s: UC list dump:\n", if_name);
			netdev_for_each_uc_addr(ha, netdev)
				pr_info("%02x:%02x:%02x:%02x:%02x:%02x\n",
					ha->addr[0], ha->addr[1],
					ha->addr[2], ha->addr[3],
					ha->addr[4], ha->addr[5]);
		}

	} else {
		printk("%s: illegal operation <%s>\n", __func__, attr->attr.name);
	}

error:
	if (err)
		printk("%s: error %d\n", __func__, err);
	dev_put(netdev);
	return err ? -EINVAL : len;
}

static ssize_t mv_debug_phy(struct device *dev,
				  struct device_attribute *attr, const char *buf, size_t len)
{
	const char      *name = attr->attr.name;
	unsigned int     err = 0, a = 0, b = 0;
	char		if_name[10];
	struct net_device *netdev;
	int val;
	struct mv_pp2x_port *port;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	sscanf(buf, "%s %x %x", if_name, &a, &b);

	netdev = dev_get_by_name(&init_net, if_name);
	if (!netdev) {
		printk("%s: illegal interface <%s>\n", __func__, if_name);
		return -EINVAL;
	}

	port = netdev_priv(netdev);

	if (!port->mac_data.phy_dev){
		printk("%s: No phy on interface <%s>\n", __func__, if_name);
		err = 1;
		goto error;
	}

	if (!strcmp(name, "phy_reg_write")) {
		phy_write(port->mac_data.phy_dev, a, b);
		val = phy_read(port->mac_data.phy_dev, a);
		printk("Interface %s: phy register %x write %x\n", if_name, a, val);
	} else if (!strcmp(name, "phy_reg_read")) {
		val = phy_read(port->mac_data.phy_dev, a);
		printk("Interface %s: phy register %x read %x\n", if_name, a, val);
	} else {
		printk("%s: illegal operation <%s>\n", __func__, attr->attr.name);
	}

error:
	if (err)
		printk("%s: error %d\n", __func__, err);
	dev_put(netdev);
	return err ? -EINVAL : len;
}


static DEVICE_ATTR(help,		S_IRUSR, mv_debug_show, NULL);
static DEVICE_ATTR(debug_param,	(S_IRUSR|S_IWUSR), mv_debug_show,
		   mv_debug_store_unsigned);
static DEVICE_ATTR(bind_cpu,		S_IWUSR, NULL, mv_debug_store);
static DEVICE_ATTR(mv_pp2x_reg_read,	S_IWUSR, NULL, mv_debug_store_unsigned);
static DEVICE_ATTR(mv_pp2x_reg_write,	S_IWUSR, NULL, mv_debug_store_unsigned);
static DEVICE_ATTR(mv_pp2x_reg_percpu_read,	S_IWUSR, NULL, mv_debug_store_unsigned);
static DEVICE_ATTR(mv_pp2x_reg_percpu_write,	S_IWUSR, NULL, mv_debug_store_unsigned);
static DEVICE_ATTR(cpn_index,		S_IWUSR, NULL, mv_debug_store_unsigned);
static DEVICE_ATTR(uc_filter_add,	S_IWUSR, NULL, mv_debug_store_mac);
static DEVICE_ATTR(uc_filter_del,	S_IWUSR, NULL, mv_debug_store_mac);
static DEVICE_ATTR(uc_filter_flush,	S_IWUSR, NULL, mv_debug_store_mac);
static DEVICE_ATTR(uc_filter_dump,	S_IWUSR, NULL, mv_debug_store_mac);
static DEVICE_ATTR(phy_reg_write,	S_IWUSR, NULL, mv_debug_phy);
static DEVICE_ATTR(phy_reg_read,		S_IWUSR, NULL, mv_debug_phy);

static struct attribute *debug_attrs[] = {
	&dev_attr_help.attr,
	&dev_attr_debug_param.attr,
	&dev_attr_bind_cpu.attr,
	&dev_attr_mv_pp2x_reg_read.attr,
	&dev_attr_mv_pp2x_reg_write.attr,
	&dev_attr_mv_pp2x_reg_percpu_read.attr,
	&dev_attr_mv_pp2x_reg_percpu_write.attr,
	&dev_attr_cpn_index.attr,
	&dev_attr_uc_filter_add.attr,
	&dev_attr_uc_filter_del.attr,
	&dev_attr_uc_filter_flush.attr,
	&dev_attr_uc_filter_dump.attr,
	&dev_attr_phy_reg_write.attr,
	&dev_attr_phy_reg_read.attr,
	NULL
};

static struct attribute_group debug_group = {
	.name = "debug",
	.attrs = debug_attrs,
};

int mv_pp2_debug_sysfs_init(struct kobject *pp2_kobj)
{
	int err = 0;

	err = sysfs_create_group(pp2_kobj, &debug_group);
	if (err)
		printk("sysfs group %s failed %d\n", debug_group.name, err);

	return err;
}

int mv_pp2_debug_sysfs_exit(struct kobject *pp2_kobj)
{
	sysfs_remove_group(pp2_kobj, &debug_group);

	return 0;
}


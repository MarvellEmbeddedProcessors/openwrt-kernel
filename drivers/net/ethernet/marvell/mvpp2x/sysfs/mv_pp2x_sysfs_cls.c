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

static struct mv_pp2x_cls_lookup_entry lkp_entry;
static struct mv_pp2x_cls_flow_entry flow_entry;


static ssize_t mv_cls_help(char *buf)
{
	int off = 0;

	off += scnprintf(buf + off, PAGE_SIZE,  "cat             lkp_sw_dump          - dump lookup ID table sw entry.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "cat             flow_sw_dump         - dump flow table SW entry.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "cat             lkp_hw_dump          - dump lookup ID tabel from hardware.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "cat             flow_hw_hits         - dump non zeroed hit counters  and the associated flow tabel entries from hardware.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "cat             lkp_hw_hits          - dump non zeroed hit counters and the associated lookup ID entires from hardware.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "cat             flow_hw_dump         - dump flow table from hardware.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "cat             hw_regs              - dump classifier top registers.\n");

	off += scnprintf(buf + off, PAGE_SIZE,  "\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo 1          >lkp_sw_clear        - clear lookup ID table SW entry.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo 1          >flow_sw_clear       - clear flow table SW entry.\n");

	off += scnprintf(buf + off, PAGE_SIZE,  "\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo en         >hw_enable           - classifier enable/disable <en = 1/0>.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo p w        >hw_port_way         - set lookup way <w> for physical port <p>.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo a b c d    >hw_udf              - set UDF field <a> as: base <b>, offset <c> bits, size<d> bits.\n");

	off += scnprintf(buf + off, PAGE_SIZE,  "echo p q        >hw_over_rxq_low     - set oversize rx low queue <q> for ingress port <p>.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo idx way    >lkp_hw_write        - write lookup ID table SW entry HW <idx,way>.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo idx way    >lkp_hw_read         - read lookup ID table entry from HW <idx,way>.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo rxq        >lkp_sw_rxq          - set default RXQ <rxq> to lookup ID table.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo f          >lkp_sw_flow         - set index of firs insruction <f> in flow table\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "                                       to lookup ID SW entry.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo m          >lkp_sw_mod          - set modification instruction offset <m> to lookup ID SW entry.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo e          >lkp_sw_en           - Enable <e=1> or disable <e=0> lookup ID table SW entry.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo id         >flow_hw_write       - write flow table SW entry to HW <id>.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo id         >flow_hw_read        - read flow table entry <id> from HW.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo t id       >flow_sw_port        - set port type <t> and id <p> to flow table SW entry\n");

	off += scnprintf(buf + off, PAGE_SIZE,  "echo from       >flow_sw_portid      - set cls to recive portid via packet <from=1>\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "                                       or via user configurration <from=0>  to flow table SW entry.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo mode       >flow_sw_pppoe       - Set PPPoE lookup skip mode <mode> to flow table SW entry.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo mode       >flow_sw_vlan        - Set VLAN lookup skip mode <mode> to flow table SW entry.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo mode       >flow_sw_macme       - Set MAC ME lookup skip mode <mode> to flow table SW entry.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo mode       >flow_sw_udf7        - Set UDF7 lookup skip mode <mode> to flow table SW entry.\n");

	off += scnprintf(buf + off, PAGE_SIZE,  "echo mode       >flow_sw_sq          - Set sequence type <mode> to flow table SW entry.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo e l        >flow_sw_engin       - set engine <e> nember to flow table SW entry.  <l> - last bit.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo l p        >flow_sw_extra       - set lookup type <l> and priority <p> to flow table SW entry.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo idx id     >flow_sw_hek         - set HEK field <idx, id> flow table SW entry.\n");
	off += scnprintf(buf + off, PAGE_SIZE,  "echo n          >flow_sw_num_of_heks - set number of HEK fields <n> to flow table SW entry.\n");

	off += scnprintf(buf + off, PAGE_SIZE,  "\n");

	return off;
}

static ssize_t mv_cls_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	const char      *name = attr->attr.name;
	int             off = 0;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (!strcmp(name, "lkp_hw_hits"))
		mv_pp2x_cls_hw_lkp_hits_dump(sysfs_cur_hw);
	else if (!strcmp(name, "flow_hw_hits"))
		mv_pp2x_cls_hw_flow_hits_dump(sysfs_cur_hw);
	else if (!strcmp(name, "lkp_hw_dump"))
		mv_pp2x_cls_hw_lkp_dump(sysfs_cur_hw);
	else if (!strcmp(name, "flow_hw_dump"))
		mv_pp2x_cls_hw_flow_dump(sysfs_cur_hw);
	else if (!strcmp(name, "hw_regs"))
		mv_pp2x_cls_hw_regs_dump(sysfs_cur_hw);
	else if (!strcmp(name, "flow_sw_dump"))
		mv_pp2x_cls_sw_flow_dump(&flow_entry);
	else if (!strcmp(name, "lkp_sw_dump"))
		mv_pp2x_cls_sw_lkp_dump(&lkp_entry);
	else
		off += mv_cls_help(buf);

	return off;
}



static ssize_t mv_cls_store_unsigned(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t len)
{
	const char    *name = attr->attr.name;
	unsigned int  err = 0, a = 0, b = 0, c = 0, d = 0;
	unsigned long flags;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	sscanf(buf, "%x %x %x %x", &a, &b, &c, &d);

	local_irq_save(flags);

	if (!strcmp(name, "lkp_hw_read"))
		mv_pp2x_cls_hw_lkp_read(sysfs_cur_hw, a, b, &lkp_entry);
	else if (!strcmp(name, "flow_hw_read"))
		mv_pp2x_cls_hw_flow_read(sysfs_cur_hw, a, &flow_entry);
	else if (!strcmp(name, "lkp_sw_clear"))
		memset(&lkp_entry, 0, sizeof(struct mv_pp2x_cls_lookup_entry));
	else if (!strcmp(name, "lkp_hw_write"))
		mv_pp2x_cls_hw_lkp_write(sysfs_cur_hw, a, b, &lkp_entry);
	else if (!strcmp(name, "lkp_sw_rxq"))
		mv_pp2x_cls_sw_lkp_rxq_set(&lkp_entry, a);
	else if (!strcmp(name, "lkp_sw_flow"))
		mv_pp2x_cls_sw_lkp_flow_set(&lkp_entry, a);
	else if (!strcmp(name, "lkp_sw_mod"))
		mv_pp2x_cls_sw_lkp_mod_set(&lkp_entry, a);
	else if (!strcmp(name, "lkp_sw_en"))
		mv_pp2x_cls_sw_lkp_en_set(&lkp_entry, a);
	else if (!strcmp(name, "flow_sw_clear"))
		memset(&flow_entry, 0, sizeof(struct mv_pp2x_cls_flow_entry));
	else if (!strcmp(name, "flow_hw_write")) {
		flow_entry.index = a;
		mv_pp2x_cls_flow_write(sysfs_cur_hw, &flow_entry);
	} else if (!strcmp(name, "flow_sw_port"))
		mv_pp2x_cls_sw_flow_port_set(&flow_entry, a, b);
	else if (!strcmp(name, "flow_sw_portid"))
		mv_pp2x_cls_sw_flow_portid_select(&flow_entry, a);
	else if (!strcmp(name, "flow_sw_pppoe"))
		mv_pp2x_cls_sw_flow_pppoe_set(&flow_entry, a);
	else if (!strcmp(name, "flow_sw_vlan"))
		mv_pp2x_cls_sw_flow_vlan_set(&flow_entry, a);
	else if (!strcmp(name, "flow_sw_macme"))
		mv_pp2x_cls_sw_flow_macme_set(&flow_entry, a);
	else if (!strcmp(name, "flow_sw_udf7"))
		mv_pp2x_cls_sw_flow_udf7_set(&flow_entry, a);
	else if (!strcmp(name, "flow_sw_sq"))
		mv_pp2x_cls_sw_flow_seq_ctrl_set(&flow_entry, a);
	else if (!strcmp(name, "flow_sw_engine"))
		mv_pp2x_cls_sw_flow_engine_set(&flow_entry, a, b);
	else if (!strcmp(name, "flow_sw_extra"))
		mv_pp2x_cls_sw_flow_extra_set(&flow_entry, a, b);
	else if (!strcmp(name, "flow_sw_hek"))
		mv_pp2x_cls_sw_flow_hek_set(&flow_entry, a, b);
	else if (!strcmp(name, "flow_sw_num_of_heks"))
		mv_pp2x_cls_sw_flow_hek_num_set(&flow_entry, a);
	else if (!strcmp(name, "hw_enable"))
		mv_pp2x_write(sysfs_cur_hw, MVPP2_CLS_MODE_REG, (unsigned int)a);
	else if (!strcmp(name, "hw_port_way"))
		mv_pp2x_cls_lkp_port_way_set(sysfs_cur_hw, a, b);
	else if (!strcmp(name, "hw_udf"))
		mv_pp2x_cls_hw_udf_set(sysfs_cur_hw, a, b, c, d);
	else if (!strcmp(name, "hw_mtu"))
		mv_pp2x_write(sysfs_cur_hw, MVPP2_CLS_MTU_REG(a), b);
	else if (!strcmp(name, "hw_over_rxq_low"))
		mv_pp2x_write(sysfs_cur_hw, MVPP2_CLS_OVERSIZE_RXQ_LOW_REG(a), b);
	else {
		err = 1;
		printk(KERN_ERR "%s: illegal operation <%s>\n", __func__, attr->attr.name);
	}
	local_irq_restore(flags);

	if (err)
		printk(KERN_ERR "%s: <%s>, error %d\n", __func__, attr->attr.name, err);

	return err ? -EINVAL : len;
}

static DEVICE_ATTR(lkp_hw_dump,			S_IRUSR, mv_cls_show, NULL);
static DEVICE_ATTR(lkp_hw_hits,			S_IRUSR, mv_cls_show, NULL);
static DEVICE_ATTR(flow_hw_hits,		S_IRUSR, mv_cls_show, NULL);
static DEVICE_ATTR(flow_hw_dump,		S_IRUSR, mv_cls_show, NULL);
static DEVICE_ATTR(lkp_sw_dump,			S_IRUSR, mv_cls_show, NULL);
static DEVICE_ATTR(flow_sw_dump,		S_IRUSR, mv_cls_show, NULL);
static DEVICE_ATTR(help,			S_IRUSR, mv_cls_show, NULL);
static DEVICE_ATTR(hw_regs,			S_IRUSR, mv_cls_show, NULL);
static DEVICE_ATTR(lkp_sw_clear,		S_IWUSR, mv_cls_show, mv_cls_store_unsigned);
static DEVICE_ATTR(lkp_hw_write,		S_IWUSR, mv_cls_show, mv_cls_store_unsigned);
static DEVICE_ATTR(lkp_hw_read,			S_IWUSR, mv_cls_show, mv_cls_store_unsigned);
static DEVICE_ATTR(lkp_sw_rxq,			S_IWUSR, mv_cls_show, mv_cls_store_unsigned);
static DEVICE_ATTR(lkp_sw_flow,			S_IWUSR, mv_cls_show, mv_cls_store_unsigned);
static DEVICE_ATTR(lkp_sw_mod,			S_IWUSR, mv_cls_show, mv_cls_store_unsigned);
static DEVICE_ATTR(lkp_sw_en,			S_IWUSR, mv_cls_show, mv_cls_store_unsigned);
static DEVICE_ATTR(flow_sw_clear,		S_IWUSR, mv_cls_show, mv_cls_store_unsigned);
static DEVICE_ATTR(flow_hw_write,		S_IWUSR, mv_cls_show, mv_cls_store_unsigned);
static DEVICE_ATTR(flow_hw_read,		S_IWUSR, mv_cls_show, mv_cls_store_unsigned);
static DEVICE_ATTR(flow_sw_port,		S_IWUSR, mv_cls_show, mv_cls_store_unsigned);
static DEVICE_ATTR(flow_sw_portid,		S_IWUSR, mv_cls_show, mv_cls_store_unsigned);
static DEVICE_ATTR(flow_sw_pppoe,		S_IWUSR, mv_cls_show, mv_cls_store_unsigned);
static DEVICE_ATTR(flow_sw_macme,		S_IWUSR, mv_cls_show, mv_cls_store_unsigned);
static DEVICE_ATTR(flow_sw_vlan,		S_IWUSR, mv_cls_show, mv_cls_store_unsigned);
static DEVICE_ATTR(flow_sw_udf7,		S_IWUSR, mv_cls_show, mv_cls_store_unsigned);
static DEVICE_ATTR(flow_sw_sq,			S_IWUSR, mv_cls_show, mv_cls_store_unsigned);
static DEVICE_ATTR(flow_sw_engine,		S_IWUSR, mv_cls_show, mv_cls_store_unsigned);
static DEVICE_ATTR(flow_sw_extra,		S_IWUSR, mv_cls_show, mv_cls_store_unsigned);
static DEVICE_ATTR(flow_sw_hek,			S_IWUSR, mv_cls_show, mv_cls_store_unsigned);
static DEVICE_ATTR(flow_sw_num_of_heks,		S_IWUSR, mv_cls_show, mv_cls_store_unsigned);
static DEVICE_ATTR(hw_enable,			S_IWUSR, mv_cls_show, mv_cls_store_unsigned);
static DEVICE_ATTR(hw_port_way,			S_IWUSR, mv_cls_show, mv_cls_store_unsigned);
static DEVICE_ATTR(hw_udf,			S_IWUSR, mv_cls_show, mv_cls_store_unsigned);
static DEVICE_ATTR(hw_mtu,			S_IWUSR, mv_cls_show, mv_cls_store_unsigned);
static DEVICE_ATTR(hw_over_rxq_low,		S_IWUSR, mv_cls_show, mv_cls_store_unsigned);

static struct attribute *cls_attrs[] = {
	&dev_attr_lkp_sw_dump.attr,
	&dev_attr_flow_sw_dump.attr,
	&dev_attr_lkp_hw_hits.attr,
	&dev_attr_flow_hw_hits.attr,
	&dev_attr_lkp_hw_dump.attr,
	&dev_attr_flow_hw_dump.attr,
	&dev_attr_hw_regs.attr,
	&dev_attr_lkp_sw_clear.attr,
	&dev_attr_lkp_hw_write.attr,
	&dev_attr_lkp_hw_read.attr,
	&dev_attr_lkp_sw_rxq.attr,
	&dev_attr_lkp_sw_flow.attr,
	&dev_attr_lkp_sw_mod.attr,
	&dev_attr_lkp_sw_en.attr,
	&dev_attr_flow_sw_clear.attr,
	&dev_attr_flow_hw_write.attr,
	&dev_attr_flow_hw_read.attr,
	&dev_attr_flow_sw_port.attr,
	&dev_attr_flow_sw_portid.attr,
	&dev_attr_flow_sw_engine.attr,
	&dev_attr_flow_sw_vlan.attr,
	&dev_attr_flow_sw_pppoe.attr,
	&dev_attr_flow_sw_macme.attr,
	&dev_attr_flow_sw_sq.attr,
	&dev_attr_flow_sw_udf7.attr,
	&dev_attr_flow_sw_extra.attr,
	&dev_attr_flow_sw_hek.attr,
	&dev_attr_flow_sw_num_of_heks.attr,
	&dev_attr_hw_enable.attr,
	&dev_attr_hw_port_way.attr,
	&dev_attr_hw_udf.attr,
	&dev_attr_hw_mtu.attr,
	&dev_attr_hw_over_rxq_low.attr,
	&dev_attr_help.attr,
	NULL
};

static struct attribute_group cls_group = {
	.name = "cls",
	.attrs = cls_attrs,
};

int mv_pp2_cls_sysfs_init(struct kobject *pp2_kobj)
{
	int err = 0;

	err = sysfs_create_group(pp2_kobj, &cls_group);
	if (err)
		printk(KERN_INFO "sysfs group %s failed %d\n", cls_group.name, err);

	return err;
}

int mv_pp2_cls_sysfs_exit(struct kobject *pp2_kobj)
{
	sysfs_remove_group(pp2_kobj, &cls_group);

	return 0;
}

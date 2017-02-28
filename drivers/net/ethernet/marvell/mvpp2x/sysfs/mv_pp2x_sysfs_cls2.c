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

struct mv_pp2x_cls_c2_qos_entry qos_entry;
struct mv_pp2x_cls_c2_entry c2_entry;

static ssize_t mv_cls2_help(char *buf)
{
	int off = 0;

	off += scnprintf(buf + off, PAGE_SIZE, "cat  prio_hw_dump - dump all QoS priority tables from HW.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "cat  dscp_hw_dump - dump all QoS dscp tables from HW.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "cat  act_hw_dump  - dump all action table enrties from HW.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "cat  hw_regs      - dump classifier C2 registers.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "cat  cnt_dump     - dump all hit counters that are not zeroed.\n");

	off += scnprintf(buf + off, PAGE_SIZE, "echo 1             > qos_sw_clear           - clear QoS table SW entry.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "echo 1             > act_sw_clear           - clear action table SW entry.\n");

	off += scnprintf(buf + off, PAGE_SIZE, "\n");
	off += scnprintf(buf + off, PAGE_SIZE, "echo id s ln       > qos_hw_write           - write QoS table SW entry into HW <id,s,ln>.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "echo id s ln       > qos_hw_read            - read QoS table entry from HW <id,s,ln>.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "echo prio          > qos_sw_prio            - set priority <prio> value to QoS table SW entry.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "echo dscp          > qos_sw_dscp            - set DSCP <dscp> value to QoS table SW entry.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "echo color         > qos_sw_color           - set color value to QoS table SW entry.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "echo q             > qos_sw_queue           - set queue number <q> value to QoS table SW entry.\n");

	off += scnprintf(buf + off, PAGE_SIZE, "\n");
	off += scnprintf(buf + off, PAGE_SIZE, "echo idx           > act_hw_write           - write action table SW entry into HW <idx>.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "echo idx           > act_hw_read            - read action table entry from HW <idx> into SW entry.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "echo idx           > act_hw_inv             - invalidate C2 entry <idx> in hw.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "echo               > act_hw_inv_all         - invalidate all C2 entries in HW.\n");

	off += scnprintf(buf + off, PAGE_SIZE, "echo o d m         > act_sw_byte            - set byte <d,m> to TCAM offset <o> to action table SW entry.\n");

	off += scnprintf(buf + off, PAGE_SIZE, "echo id sel        > act_sw_qos             - set QoS table <id,sel> to action table SW entry.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "echo cmd from      > act_sw_color           - set color command <cmd> to action table SW.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "                                              <from> - source for color command.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "echo cmd prio from > act_sw_prio            - set priority command <cmd> and value <prio> to action\n");
	off += scnprintf(buf + off, PAGE_SIZE, "                                              table SW entry. <from> - source for priority command.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "echo cmd prio from > act_sw_dscp            - set DSCP command <cmd> and value <dscp> to action\n");
	off += scnprintf(buf + off, PAGE_SIZE, "                                              table SW entry. <from> - source for DSCP command.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "echo cmd q from    > act_sw_qh              - set queue high command <cmd> and value <q> to action\n");
	off += scnprintf(buf + off, PAGE_SIZE, "                                              table software entry. <from>-source for Queue High command.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "echo cmd q from    > act_sw_ql              - set queue low command <cmd> and value <q> to action\n");
	off += scnprintf(buf + off, PAGE_SIZE, "                                              table software entry. <from> -source for Queue Low command.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "echo cmd q from    > act_sw_queue           - set full queue command <cmd> and value <q> to action\n");
	off += scnprintf(buf + off, PAGE_SIZE, "                                              table software entry.  <from> -source for Queue command.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "echo cmd           > act_sw_hwf             - set Forwarding command <cmd> to action table SW entry.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "echo cmd id bank   > act_sw_rss             - set RSS command <cmd> and enable RSS.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "echo en            > act_sw_flowid          - set FlowID enable/disable <1/0> to action table SW entry.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "echo idx           > act_sw_mtu             - set MTU index to action table SW entry\n");
	off += scnprintf(buf + off, PAGE_SIZE, "echo id cnt        > act_sw_dup             - set packet duplication parameters <id,cnt> to action table SW entry.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "echo d i cs        > act_sw_mdf             - set modification parameters to action table SW entry\n");
	off += scnprintf(buf + off, PAGE_SIZE, "                                              data pointer <d>, instruction pointrt <i>,\n");
	off += scnprintf(buf + off, PAGE_SIZE, "                                              <cs> enable L4 checksum generation.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "\n");
	off += scnprintf(buf + off, PAGE_SIZE, "echo               > cnt_clr_all            - clear all hit counters from action tabe.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "echo idx           > cnt_read               - show hit counter for action table entry.\n");
	off += scnprintf(buf + off, PAGE_SIZE, "\n");
	return off;
}


static ssize_t mv_cls2_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	const char      *name = attr->attr.name;
	int             off = 0;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (!strcmp(name, "prio_hw_dump"))
		off += mv_pp2x_cls_c2_qos_prio_hw_dump(sysfs_cur_hw);
	else if (!strcmp(name, "dscp_hw_dump"))
		off += mv_pp2x_cls_c2_qos_dscp_hw_dump(sysfs_cur_hw);
	else if (!strcmp(name, "act_hw_dump"))
		off += mv_pp2x_cls_c2_hw_dump(sysfs_cur_hw);
	else if (!strcmp(name, "cnt_dump"))
		off += mv_pp2x_cls_c2_hit_cntr_dump(sysfs_cur_hw);
	else if (!strcmp(name, "hw_regs"))
		off += mv_pp2x_cls_c2_regs_dump(sysfs_cur_hw);
	else
		off += mv_cls2_help(buf);

	return off;
}


static ssize_t mv_cls2_store(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t len)
{
	const char    *name = attr->attr.name;
	unsigned int  err = 0, a = 0, b = 0, c = 0, d = 0, e = 0;
	unsigned long flags;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	sscanf(buf, "%x %x %x %x %x", &a, &b, &c, &d, &e);

	local_irq_save(flags);

	if (!strcmp(name, "cnt_clr_all"))
		mv_pp2x_cls_c2_hit_cntr_clear_all(sysfs_cur_hw);
	else if (!strcmp(name, "cnt_read"))
		mv_pp2x_cls_c2_hit_cntr_read(sysfs_cur_hw, a, NULL);
	else if (!strcmp(name, "act_hw_inv_all"))
		mv_pp2x_cls_c2_hw_inv_all(sysfs_cur_hw);
	else if (!strcmp(name, "act_hw_inv"))
		mv_pp2x_cls_c2_hw_inv(sysfs_cur_hw, a);
	else if (!strcmp(name, "qos_sw_clear"))
		memset(&qos_entry, 0, sizeof(struct mv_pp2x_cls_c2_qos_entry));
	else if (!strcmp(name, "qos_hw_write")) {
		qos_entry.tbl_id = a;
		qos_entry.tbl_sel = b;
		qos_entry.tbl_line = c;
		mv_pp2x_cls_c2_qos_hw_write(sysfs_cur_hw, &qos_entry);
	} else if (!strcmp(name, "qos_hw_read"))
		mv_pp2x_cls_c2_qos_hw_read(sysfs_cur_hw, a, b, c, &qos_entry);
	else if (!strcmp(name, "qos_sw_prio"))
		mv_pp2x_cls_c2_qos_prio_set(&qos_entry, a);
	else if (!strcmp(name, "qos_sw_dscp"))
		mv_pp2x_cls_c2_qos_dscp_set(&qos_entry, a);
	else if (!strcmp(name, "qos_sw_color"))
		mv_pp2x_cls_c2_qos_color_set(&qos_entry, a);
	else if (!strcmp(name, "qos_sw_queue"))
		mv_pp2x_cls_c2_qos_queue_set(&qos_entry, a);
	else if (!strcmp(name, "act_sw_clear"))
		memset(&c2_entry, 0, sizeof(struct mv_pp2x_c2_add_entry));
	else if (!strcmp(name, "act_hw_write"))
		mv_pp2x_cls_c2_hw_write(sysfs_cur_hw, a, &c2_entry);
	else if (!strcmp(name, "act_hw_read"))
		mv_pp2x_cls_c2_hw_read(sysfs_cur_hw, a, &c2_entry);
	else if (!strcmp(name, "act_sw_byte"))
		mv_pp2x_cls_c2_tcam_byte_set(&c2_entry, a, b, c);
	else if (!strcmp(name, "act_sw_qos"))
		mv_pp2x_cls_c2_qos_tbl_set(&c2_entry, a, b);
	else if (!strcmp(name, "act_sw_color"))
		mv_pp2x_cls_c2_color_set(&c2_entry, a, b);
	else if (!strcmp(name, "act_sw_prio"))
		mv_pp2x_cls_c2_prio_set(&c2_entry, a, b, c);
	else if (!strcmp(name, "act_sw_dscp"))
		mv_pp2x_cls_c2_dscp_set(&c2_entry, a, b, c);
	else if (!strcmp(name, "act_sw_qh"))
		mv_pp2x_cls_c2_queue_high_set(&c2_entry, a, b, c);
	else if (!strcmp(name, "act_sw_ql"))
		mv_pp2x_cls_c2_queue_low_set(&c2_entry, a, b, c);
	else if (!strcmp(name, "act_sw_queue"))
		mv_pp2x_cls_c2_queue_set(&c2_entry, a, b, c);
	else if (!strcmp(name, "act_sw_hwf"))
		mv_pp2x_cls_c2_forward_set(&c2_entry, a);
	else if (!strcmp(name, "act_sw_rss"))
		mv_pp2x_cls_c2_rss_set(&c2_entry, a, b);
	else if (!strcmp(name, "act_sw_mtu"))
		mv_pp2x_cls_c2_mtu_set(&c2_entry, a);
	else if (!strcmp(name, "act_sw_flowid"))
		mv_pp2x_cls_c2_flow_id_en(&c2_entry, a);
	else if (!strcmp(name, "act_sw_dup"))
		mv_pp2x_cls_c2_dup_set(&c2_entry, a, b);
	else if (!strcmp(name, "act_sw_mdf"))
		mv_pp2x_cls_c2_mod_set(&c2_entry, a, b, c);
	else {
		err = 1;
		printk(KERN_ERR "%s: illegal operation <%s>\n", __func__, attr->attr.name);
	}
	local_irq_restore(flags);

	if (err)
		printk(KERN_ERR "%s: <%s>, error %d\n", __func__, attr->attr.name, err);

	return err ? -EINVAL : len;
}


static DEVICE_ATTR(prio_hw_dump,	S_IRUSR, mv_cls2_show, NULL);
static DEVICE_ATTR(dscp_hw_dump,	S_IRUSR, mv_cls2_show, NULL);
static DEVICE_ATTR(act_hw_dump,		S_IRUSR, mv_cls2_show, NULL);
static DEVICE_ATTR(cnt_dump,		S_IRUSR, mv_cls2_show, NULL);
static DEVICE_ATTR(hw_regs,		S_IRUSR, mv_cls2_show, NULL);
static DEVICE_ATTR(help,		S_IRUSR, mv_cls2_show, NULL);

static DEVICE_ATTR(qos_sw_clear,	S_IWUSR, mv_cls2_show, mv_cls2_store);
static DEVICE_ATTR(qos_hw_write,	S_IWUSR, mv_cls2_show, mv_cls2_store);
static DEVICE_ATTR(qos_hw_read,		S_IWUSR, mv_cls2_show, mv_cls2_store);
static DEVICE_ATTR(qos_sw_prio,		S_IWUSR, mv_cls2_show, mv_cls2_store);
static DEVICE_ATTR(qos_sw_dscp,		S_IWUSR, mv_cls2_show, mv_cls2_store);
static DEVICE_ATTR(qos_sw_color,	S_IWUSR, mv_cls2_show, mv_cls2_store);
static DEVICE_ATTR(qos_sw_queue,	S_IWUSR, mv_cls2_show, mv_cls2_store);
static DEVICE_ATTR(act_hw_inv,		S_IWUSR, mv_cls2_show, mv_cls2_store);
static DEVICE_ATTR(act_hw_inv_all,	S_IWUSR, mv_cls2_show, mv_cls2_store);
static DEVICE_ATTR(act_sw_clear,	S_IWUSR, mv_cls2_show, mv_cls2_store);
static DEVICE_ATTR(act_hw_write,	S_IWUSR, mv_cls2_show, mv_cls2_store);
static DEVICE_ATTR(act_hw_read,		S_IWUSR, mv_cls2_show, mv_cls2_store);
static DEVICE_ATTR(act_sw_byte,		S_IWUSR, mv_cls2_show, mv_cls2_store);
static DEVICE_ATTR(act_sw_color,	S_IWUSR, mv_cls2_show, mv_cls2_store);
static DEVICE_ATTR(act_sw_prio,		S_IWUSR, mv_cls2_show, mv_cls2_store);
static DEVICE_ATTR(act_sw_dscp,		S_IWUSR, mv_cls2_show, mv_cls2_store);
static DEVICE_ATTR(act_sw_qh,		S_IWUSR, mv_cls2_show, mv_cls2_store);
static DEVICE_ATTR(act_sw_ql,		S_IWUSR, mv_cls2_show, mv_cls2_store);
static DEVICE_ATTR(act_sw_queue,	S_IWUSR, mv_cls2_show, mv_cls2_store);
static DEVICE_ATTR(act_sw_hwf,		S_IWUSR, mv_cls2_show, mv_cls2_store);
static DEVICE_ATTR(act_sw_rss,		S_IWUSR, mv_cls2_show, mv_cls2_store);
static DEVICE_ATTR(act_sw_mtu,		S_IWUSR, mv_cls2_show, mv_cls2_store);
static DEVICE_ATTR(cnt_clr_all,		S_IWUSR, mv_cls2_show, mv_cls2_store);
static DEVICE_ATTR(act_sw_qos,		S_IWUSR, mv_cls2_show, mv_cls2_store);
static DEVICE_ATTR(cnt_read,		S_IWUSR, mv_cls2_show, mv_cls2_store);
static DEVICE_ATTR(act_sw_flowid,	S_IWUSR, mv_cls2_show, mv_cls2_store);
static DEVICE_ATTR(act_sw_dup,		S_IWUSR, mv_cls2_show, mv_cls2_store);
static DEVICE_ATTR(act_sw_mdf,		S_IWUSR, mv_cls2_show, mv_cls2_store);

static struct attribute *cls2_attrs[] = {
	&dev_attr_prio_hw_dump.attr,
	&dev_attr_dscp_hw_dump.attr,
	&dev_attr_act_hw_dump.attr,
	&dev_attr_cnt_dump.attr,
	&dev_attr_hw_regs.attr,
	&dev_attr_help.attr,
	&dev_attr_qos_sw_clear.attr,
	&dev_attr_qos_hw_write.attr,
	&dev_attr_qos_hw_read.attr,
	&dev_attr_qos_sw_prio.attr,
	&dev_attr_qos_sw_dscp.attr,
	&dev_attr_qos_sw_color.attr,
	&dev_attr_qos_sw_queue.attr,
	&dev_attr_act_hw_inv.attr,
	&dev_attr_act_hw_inv_all.attr,
	&dev_attr_act_sw_clear.attr,
	&dev_attr_act_hw_write.attr,
	&dev_attr_act_hw_read.attr,
	&dev_attr_act_sw_byte.attr,
	&dev_attr_act_sw_color.attr,
	&dev_attr_act_sw_prio.attr,
	&dev_attr_act_sw_dscp.attr,
	&dev_attr_act_sw_qh.attr,
	&dev_attr_act_sw_ql.attr,
	&dev_attr_act_sw_queue.attr,
	&dev_attr_act_sw_hwf.attr,
	&dev_attr_act_sw_rss.attr,
	&dev_attr_act_sw_mtu.attr,
	&dev_attr_cnt_clr_all.attr,
	&dev_attr_act_sw_qos.attr,
	&dev_attr_cnt_read.attr,
	&dev_attr_act_sw_flowid.attr,
	&dev_attr_act_sw_dup.attr,
	&dev_attr_act_sw_mdf.attr,
	NULL
};

static struct attribute_group cls2_group = {
	.name = "cls2",
	.attrs = cls2_attrs,
};

int mv_pp2_cls2_sysfs_init(struct kobject *pp2_kobj)
{
	int err = 0;

	err = sysfs_create_group(pp2_kobj, &cls2_group);
	if (err)
		printk(KERN_INFO "sysfs group %s failed %d\n", cls2_group.name, err);

	return err;
}

int mv_pp2_cls2_sysfs_exit(struct kobject *pp2_kobj)
{
	sysfs_remove_group(pp2_kobj, &cls2_group);
	return 0;
}


/*
* ***************************************************************************
* Copyright (C) 2016 Marvell International Ltd.
* ***************************************************************************
* This program is free software: you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the Free
* Software Foundation, either version 2 of the License, or any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* ***************************************************************************
*/

#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/platform_device.h>
#include <linux/skbuff.h>
#include <linux/inetdevice.h>
#include <linux/mbus.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/cpumask.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_mdio.h>
#include <linux/of_net.h>
#include <linux/of_address.h>
#include <linux/of_device.h>

#include <linux/phy.h>
#include <linux/clk.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <uapi/linux/ppp_defs.h>
#include <net/ip.h>
#include <net/ipv6.h>

#include "mv_pp2x.h"
#include "mv_pp2x_hw.h"
#include "mv_pp2x_sysfs_debug.h"



/* mv_pp2x_port_bind_cpu_set
*  -- Bind the port to cpu when rss disabled.
*/
int mv_pp2x_port_bind_cpu_set(struct mv_pp2x_port *port, u8 bind_cpu)
{
	int ret = 0;
	u8 bound_cpu_first_rxq;

	if (port->rss_cfg.rss_en) {
		netdev_err(port->dev,
			"cannot bind cpu to port when rss is enabled\n");
		return -EINVAL;
	}

	if (!((*cpumask_bits(cpu_online_mask)) & (1 << bind_cpu))) {
		netdev_err(port->dev, "invalid cpu(%d)\n", bind_cpu);
		return -EINVAL;
	}

	/* Check original cpu and new cpu is same or not */
	if (bind_cpu != ((port->priv->pp2_cfg.rx_cpu_map >> (port->id * 4)) &
	    0xF)) {
		port->priv->pp2_cfg.rx_cpu_map &= (~(0xF << (port->id * 4)));
		port->priv->pp2_cfg.rx_cpu_map |= ((bind_cpu & 0xF) <<
						   (port->id * 4));
		bound_cpu_first_rxq = mv_pp2x_bound_cpu_first_rxq_calc(port);
		ret = mv_pp2x_cls_c2_rule_set(port, bound_cpu_first_rxq);
	}

	return ret;
}
EXPORT_SYMBOL(mv_pp2x_port_bind_cpu_set);


/* Set maximum burst size for TX port
 *   burst [bytes] - number of bytes to be sent with maximum possible TX rate,
 *                    before TX rate limitation will take place.
 */
int mvPp2TxpBurstSet(struct mv_pp2x *priv, int port, int burst)
{
	u32 size, mtu;
	int txPortNum;
	struct mv_pp2x_hw *hw = &priv->hw;
	struct mv_pp2x_port *pp2_port = mv_pp2x_port_struct_get(priv, port);

	if (port >= MVPP2_MAX_PORTS)
		return -1;

	txPortNum = mv_pp2x_egress_port(pp2_port);
	mv_pp2x_write(hw, MVPP2_TXP_SCHED_PORT_INDEX_REG, txPortNum);

	/* Calculate Token Bucket Size */
	size = 8 * burst;

	if (size > MVPP2_TXP_TOKEN_SIZE_MAX)
		size = MVPP2_TXP_TOKEN_SIZE_MAX;

	/* Token bucket size must be larger then MTU */
	mtu = mv_pp2x_read(hw, MVPP2_TXP_SCHED_MTU_REG);
	if (mtu > size) {
		DBG_MSG("%s Error: Bucket size (%d bytes) < MTU (%d bytes)\n",
			__func__, (size / 8), (mtu / 8));
		return -1;
	}
	mv_pp2x_write(hw, MVPP2_TXP_SCHED_TOKEN_SIZE_REG, size);

	return 0;
}
EXPORT_SYMBOL(mvPp2TxpBurstSet);

/* Set bandwidth limitation for TXQ
 *   rate  [Kbps]  - steady state TX rate limitation
 */
int mvPp2TxqRateSet(struct mv_pp2x *priv, int port, int txq, int rate)
{
	u32		regVal;
	unsigned int	txPortNum, period, tokens, accuracy = 0;
	int	status;
	struct mv_pp2x_hw *hw = &priv->hw;
	struct mv_pp2x_port *pp2_port = mv_pp2x_port_struct_get(priv, port);

	if (port >= MVPP2_MAX_PORTS)
		return -1;

	if (txq >= MVPP2_MAX_TXQ)
		return -1;

	status = mvPp2RateCalc(rate, accuracy, &period, &tokens);
	if (status != MV_OK) {
		DBG_MSG(
			"%s: Can't provide rate of %d [Kbps] with accuracy of %d [%%]\n",
			__func__, rate, accuracy);
		return status;
	}

	txPortNum = mv_pp2x_egress_port(pp2_port);
	mv_pp2x_write(hw, MVPP2_TXP_SCHED_PORT_INDEX_REG, txPortNum);

	if (tokens > MVPP2_TXQ_REFILL_TOKENS_MAX)
		tokens = MVPP2_TXQ_REFILL_TOKENS_MAX;

	if (period > MVPP2_TXQ_REFILL_PERIOD_MAX)
		period = MVPP2_TXQ_REFILL_PERIOD_MAX;

	regVal = mv_pp2x_read(hw, MVPP2_TXQ_SCHED_REFILL_REG(txq));

	regVal &= ~MVPP2_TXQ_REFILL_TOKENS_ALL_MASK;
	regVal |= MVPP2_TXQ_REFILL_TOKENS_MASK(tokens);

	regVal &= ~MVPP2_TXQ_REFILL_PERIOD_ALL_MASK;
	regVal |= MVPP2_TXQ_REFILL_PERIOD_MASK(period);

	mv_pp2x_write(hw, MVPP2_TXQ_SCHED_REFILL_REG(txq), regVal);

	return 0;
}
EXPORT_SYMBOL(mvPp2TxqRateSet);

/* Set maximum burst size for TX port
 *   burst [bytes] - number of bytes to be sent with maximum possible TX rate,
 *                    before TX bandwidth limitation will take place.
 */
int mvPp2TxqBurstSet(struct mv_pp2x *priv, int port, int txq, int burst)
{
	u32  size, mtu;
	int txPortNum;
	struct mv_pp2x_hw *hw = &priv->hw;
	struct mv_pp2x_port *pp2_port = mv_pp2x_port_struct_get(priv, port);

	if (port >= MVPP2_MAX_PORTS)
		return -1;

	if (txq >= MVPP2_MAX_TXQ)
		return -1;

	txPortNum = mv_pp2x_egress_port(pp2_port);
	mv_pp2x_write(hw, MVPP2_TXP_SCHED_PORT_INDEX_REG, txPortNum);

	/* Calculate Tocket Bucket Size */
	size = 8 * burst;

	if (size > MVPP2_TXQ_TOKEN_SIZE_MAX)
		size = MVPP2_TXQ_TOKEN_SIZE_MAX;

	/* Tocken bucket size must be larger then MTU */
	mtu = mv_pp2x_read(hw, MVPP2_TXP_SCHED_MTU_REG);
	if (mtu > size) {
		DBG_MSG(
			"%s Error: Bucket size (%d bytes) < MTU (%d bytes)\n",
			__func__, (size / 8), (mtu / 8));
		return -1;
	}

	mv_pp2x_write(hw, MVPP2_TXQ_SCHED_TOKEN_SIZE_REG(txq), size);

	return 0;
}
EXPORT_SYMBOL(mvPp2TxqBurstSet);

/* Set TXQ to work in FIX priority mode */
int mvPp2TxqFixPrioSet(struct mv_pp2x *priv, int port, int txq)
{
	u32 regVal;
	int txPortNum;
	struct mv_pp2x_hw *hw = &priv->hw;
	struct mv_pp2x_port *pp2_port = mv_pp2x_port_struct_get(priv, port);

	if (port >= MVPP2_MAX_PORTS)
		return -1;

	if (txq >= MVPP2_MAX_TXQ)
		return -1;

	txPortNum = mv_pp2x_egress_port(pp2_port);
	mv_pp2x_write(hw, MVPP2_TXP_SCHED_PORT_INDEX_REG, txPortNum);

	regVal = mv_pp2x_read(hw, MVPP2_TXP_SCHED_FIXED_PRIO_REG);
	regVal |= (1 << txq);
	mv_pp2x_write(hw, MVPP2_TXP_SCHED_FIXED_PRIO_REG, regVal);

	return MV_OK;
}
EXPORT_SYMBOL(mvPp2TxqFixPrioSet);

/* Set TXQ to work in WRR mode and set relative weight. */
/*   Weight range [1..N] */
int mvPp2TxqWrrPrioSet(struct mv_pp2x *priv, int port, int txq, int weight)
{
	u32 regVal, mtu, mtu_aligned, weight_min;
	int txPortNum;
	struct mv_pp2x_hw *hw = &priv->hw;
	struct mv_pp2x_port *pp2_port = mv_pp2x_port_struct_get(priv, port);

	if (port >= MVPP2_MAX_PORTS)
		return -1;

	if (txq >= MVPP2_MAX_TXQ)
		return -1;

	txPortNum = mv_pp2x_egress_port(pp2_port);
	mv_pp2x_write(hw, MVPP2_TXP_SCHED_PORT_INDEX_REG, txPortNum);

	/* Weight * 256 bytes * 8 bits must be larger then MTU [bits] */
	mtu = mv_pp2x_read(hw, MVPP2_TXP_SCHED_MTU_REG);

	/* WA for wrong Token bucket update: Set MTU value =
	 * 3*real MTU value, now get read MTU
	 */
	mtu /= MV_AMPLIFY_FACTOR_MTU;
	mtu /= MV_BIT_NUM_OF_BYTE; /* move to bytes */
	mtu_aligned = round_up(mtu, MV_WRR_WEIGHT_UNIT);
	weight_min = mtu_aligned / MV_WRR_WEIGHT_UNIT;

	if ((weight < weight_min) || (weight > MVPP2_TXQ_WRR_WEIGHT_MAX)) {
		DBG_MSG("%s Error: weight=%d is out of range %d...%d\n",
			__func__, weight, weight_min,
			MVPP2_TXQ_WRR_WEIGHT_MAX);
		return -1;
	}

	regVal = mv_pp2x_read(hw, MVPP2_TXQ_SCHED_WRR_REG(txq));

	regVal &= ~MVPP2_TXQ_WRR_WEIGHT_ALL_MASK;
	regVal |= MVPP2_TXQ_WRR_WEIGHT_MASK(weight);
	mv_pp2x_write(hw, MVPP2_TXQ_SCHED_WRR_REG(txq), regVal);

	regVal = mv_pp2x_read(hw, MVPP2_TXP_SCHED_FIXED_PRIO_REG);
	regVal &= ~(1 << txq);
	mv_pp2x_write(hw, MVPP2_TXP_SCHED_FIXED_PRIO_REG, regVal);

	return 0;
}
EXPORT_SYMBOL(mvPp2TxqWrrPrioSet);

int mv_pp2x_cls_c2_qos_prio_set(struct mv_pp2x_cls_c2_qos_entry *qos, u8 pri)
{
	if (!qos)
		return -EINVAL;

	qos->data &= ~MVPP2_CLS2_QOS_TBL_PRI_MASK;
	qos->data |= (((u32)pri) << MVPP2_CLS2_QOS_TBL_PRI_OFF);
	return 0;
}
EXPORT_SYMBOL(mv_pp2x_cls_c2_qos_prio_set);

int mv_pp2x_cls_c2_qos_dscp_set(struct mv_pp2x_cls_c2_qos_entry *qos, u8 dscp)
{
	if (!qos)
		return -EINVAL;

	qos->data &= ~MVPP2_CLS2_QOS_TBL_DSCP_MASK;
	qos->data |= (((u32)dscp) << MVPP2_CLS2_QOS_TBL_DSCP_OFF);
	return 0;
}
EXPORT_SYMBOL(mv_pp2x_cls_c2_qos_dscp_set);

int mv_pp2x_cls_c2_qos_color_set(struct mv_pp2x_cls_c2_qos_entry *qos, u8 color)
{
	if (!qos)
		return -EINVAL;

	qos->data &= ~MVPP2_CLS2_QOS_TBL_COLOR_MASK;
	qos->data |= (((u32)color) << MVPP2_CLS2_QOS_TBL_COLOR_OFF);
	return 0;
}
EXPORT_SYMBOL(mv_pp2x_cls_c2_qos_color_set);

int mv_pp2x_cls_c2_queue_set(struct mv_pp2x_cls_c2_entry *c2, int cmd,
		int queue, int from)
{
	int status = 0;
	int qHigh, qLow;

	/* cmd validation in set functions */

	qHigh = (queue & MVPP2_CLS2_ACT_QOS_ATTR_QH_MASK) >>
		MVPP2_CLS2_ACT_QOS_ATTR_QH_OFF;
	qLow = (queue & MVPP2_CLS2_ACT_QOS_ATTR_QL_MASK) >>
		MVPP2_CLS2_ACT_QOS_ATTR_QL_OFF;

	status |= mv_pp2x_cls_c2_queue_low_set(c2, cmd, qLow, from);
	status |= mv_pp2x_cls_c2_queue_high_set(c2, cmd, qHigh, from);

	return status;
}
EXPORT_SYMBOL(mv_pp2x_cls_c2_queue_set);

int mv_pp2x_cls_c2_mtu_set(struct mv_pp2x_cls_c2_entry *c2, int mtu_inx)
{
	if (mv_pp2x_ptr_validate(c2) == MV_ERROR)
		return MV_ERROR;

	if (mv_pp2x_range_validate(mtu_inx, 0,
	    (1 << MVPP2_CLS2_ACT_HWF_ATTR_MTUIDX_BITS) - 1) == MV_ERROR)
		return MV_ERROR;

	c2->sram.regs.hwf_attr &= ~MVPP2_CLS2_ACT_HWF_ATTR_MTUIDX_MASK;
	c2->sram.regs.hwf_attr |= (mtu_inx <<
				  MVPP2_CLS2_ACT_HWF_ATTR_MTUIDX_OFF);

	return MV_OK;
}
EXPORT_SYMBOL(mv_pp2x_cls_c2_mtu_set);

int mv_pp2x_cls_c2_dup_set(struct mv_pp2x_cls_c2_entry *c2, int dupid, int count)
{
	if (mv_pp2x_ptr_validate(c2) == MV_ERROR)
		return MV_ERROR;

	/*set flowid and count*/
	c2->sram.regs.rss_attr &= ~(MVPP2_CLS2_ACT_DUP_ATTR_DUPID_MASK | MVPP2_CLS2_ACT_DUP_ATTR_DUPCNT_MASK);
	c2->sram.regs.rss_attr |= (dupid << MVPP2_CLS2_ACT_DUP_ATTR_DUPID_OFF);
	c2->sram.regs.rss_attr |= (count << MVPP2_CLS2_ACT_DUP_ATTR_DUPCNT_OFF);

	return MV_OK;
}
EXPORT_SYMBOL(mv_pp2x_cls_c2_dup_set);

int mv_pp2x_cls_c2_mod_set(struct mv_pp2x_cls_c2_entry *c2, int data_ptr, int instr_offs, int l4_csum)
{
	if (mv_pp2x_ptr_validate(c2) == MV_ERROR)
		return MV_ERROR;

	c2->sram.regs.hwf_attr &= ~MVPP2_CLS2_ACT_HWF_ATTR_DPTR_MASK;
	c2->sram.regs.hwf_attr &= ~MVPP2_CLS2_ACT_HWF_ATTR_IPTR_MASK;
	c2->sram.regs.hwf_attr &= ~MVPP2_CLS2_ACT_HWF_ATTR_L4CHK_MASK;

	c2->sram.regs.hwf_attr |= (data_ptr << MVPP2_CLS2_ACT_HWF_ATTR_DPTR_OFF);
	c2->sram.regs.hwf_attr |= (instr_offs << MVPP2_CLS2_ACT_HWF_ATTR_IPTR_OFF);
	c2->sram.regs.hwf_attr |= (l4_csum << MVPP2_CLS2_ACT_HWF_ATTR_L4CHK_OFF);

	return MV_OK;
}
EXPORT_SYMBOL(mv_pp2x_cls_c2_mod_set);

/* mv_pp2x_cos_classifier_get
*  -- Get the cos classifier on the port.
*/
int mv_pp2x_cos_classifier_get(struct mv_pp2x_port *port)
{
	return port->cos_cfg.cos_classifier;
}
EXPORT_SYMBOL(mv_pp2x_cos_classifier_get);


/* mv_pp2x_cos_pri_map_get
*  -- Get priority_map on the port.
*/
int mv_pp2x_cos_pri_map_get(struct mv_pp2x_port *port)
{
	return port->cos_cfg.pri_map;
}
EXPORT_SYMBOL(mv_pp2x_cos_pri_map_get);

/* mv_pp2x_cos_default_value_get
*  -- Get default cos value for untagged or non-IP packets on the port.
*/

int mv_pp2x_cos_default_value_get(struct mv_pp2x_port *port)
{
	return port->cos_cfg.default_cos;
}
EXPORT_SYMBOL(mv_pp2x_cos_default_value_get);



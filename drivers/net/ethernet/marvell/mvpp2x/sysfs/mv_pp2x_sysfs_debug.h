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

#ifndef _MVPP2_SYSFS_DEBUG_H_
#define _MVPP2_SYSFS_DEBUG_H_

#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/platform_device.h>

#define MV_AMPLIFY_FACTOR_MTU				(3)
#define MV_BIT_NUM_OF_BYTE				(8)
#define MV_WRR_WEIGHT_UNIT				(256)

#define DBG_MSG(fmt, args...)	printk(fmt, ## args)

/* This macro returns absolute value                                        */
#define MV_PP2_ABS(number)  (((int)(number) < 0) ? -(int)(number) : (int)(number))

/* Macro for alignment up. For example, MV_ALIGN_UP(0x0330, 0x20) = 0x0340   */
#define MV_PP2_ALIGN_UP(number, align) (((number) & ((align) - 1)) ? \
				    (((number) + (align)) & ~((align) - 1)) : \
				    (number))

static inline int mv_pp2x_max_check(int value, int limit, char *name)
{
	if ((value < 0) || (value >= limit)) {
		DBG_MSG("%s %d is out of range [0..%d]\n",
			name ? name : "value", value, (limit - 1));
		return 1;
	}
	return 0;
}

void mv_pp2x_print_reg(struct mv_pp2x_hw *hw, unsigned int reg_addr,
			   char *reg_name);
void mv_pp2x_print_reg2(struct mv_pp2x_hw *hw, unsigned int reg_addr,
			     char *reg_name, unsigned int index);

void mv_pp2x_bm_pool_regs(struct mv_pp2x_hw *hw, int pool);
void mv_pp2x_bm_pool_drop_count(struct mv_pp2x_hw *hw, int pool);
void mv_pp2x_pool_status(struct mv_pp2x *priv, int log_pool_num);
void mv_pp2_pool_stats_print(struct mv_pp2x *priv, int log_pool_num);

void mvPp2RxDmaRegsPrint(struct mv_pp2x *priv, bool print_all,
			 int start, int stop);
void mvPp2RxqShow(struct mv_pp2x *priv, int port, int rxq, int mode);
void mvPp2PhysRxqRegs(struct mv_pp2x *pp2, int rxq);
void mvPp2PortRxqRegs(struct mv_pp2x *pp2, int port, int rxq);
void mv_pp22_isr_rx_group_regs(struct mv_pp2x *priv, int port, bool print_all);

void mvPp2V1RxqDbgCntrs(struct mv_pp2x *priv, int port, int rxq);
void mvPp2RxFifoRegs(struct mv_pp2x_hw *hw, int port);

void mv_pp2x_rx_desc_print(struct mv_pp2x *priv, struct mv_pp2x_rx_desc *desc);

void mv_pp2x_skb_dump(struct sk_buff *skb, int size, int access);
void mvPp2TxqShow(struct mv_pp2x *priv, int port, int txq, int mode);
void mvPp2AggrTxqShow(struct mv_pp2x *priv, int cpu, int mode);
void mvPp2PhysTxqRegs(struct mv_pp2x *priv, int txq);
void mvPp2PortTxqRegs(struct mv_pp2x *priv, int port, int txq);
void mvPp2AggrTxqRegs(struct mv_pp2x *priv, int cpu);
void mvPp2V1TxqDbgCntrs(struct mv_pp2x *priv, int port, int txq);
void mvPp2V1DropCntrs(struct mv_pp2x *priv, int port);
void mvPp2TxRegs(struct mv_pp2x *priv);
void mvPp2TxSchedRegs(struct mv_pp2x *priv, int port);
int mvPp2TxpRateSet(struct mv_pp2x *priv, int port, int rate);
int mvPp2TxpBurstSet(struct mv_pp2x *priv, int port, int burst);
int mvPp2TxqRateSet(struct mv_pp2x *priv, int port, int txq, int rate);
int mvPp2TxqBurstSet(struct mv_pp2x *priv, int port, int txq, int burst);
int mvPp2TxqFixPrioSet(struct mv_pp2x *priv, int port, int txq);
int mvPp2TxqWrrPrioSet(struct mv_pp2x *priv, int port, int txq, int weight);

int mv_pp2x_port_bind_cpu_set(struct mv_pp2x_port *port, u8 bind_cpu);
int mv_pp2x_debug_param_set(u32 param);
int mv_pp2x_debug_param_get(void);

void mv_pp2x_bm_queue_map_dump_all(struct mv_pp2x_hw *hw);

int mv_pp2x_cls_c2_qos_prio_set(struct mv_pp2x_cls_c2_qos_entry *qos, u8 pri);
int mv_pp2x_cls_c2_qos_dscp_set(struct mv_pp2x_cls_c2_qos_entry *qos, u8 dscp);
int mv_pp2x_cls_c2_qos_color_set(struct mv_pp2x_cls_c2_qos_entry *qos,
				 u8 color);
int mv_pp2x_cls_c2_queue_set(struct mv_pp2x_cls_c2_entry *c2, int cmd,
			     int queue, int from);
int mv_pp2x_cls_c2_mtu_set(struct mv_pp2x_cls_c2_entry *c2, int mtu_inx);
int mv_pp2x_cls_c2_dup_set(struct mv_pp2x_cls_c2_entry *c2, int dupid, int count);
int mv_pp2x_cls_c2_mod_set(struct mv_pp2x_cls_c2_entry *c2, int data_ptr, int instr_offs, int l4_csum);

int mv_pp2x_prs_sw_dump(struct mv_pp2x_prs_entry *pe);
int mv_pp2x_prs_hw_dump(struct mv_pp2x_hw *hw);
int mv_pp2x_prs_hw_regs_dump(struct mv_pp2x_hw *hw);
int mv_pp2x_prs_hw_hits_dump(struct mv_pp2x_hw *hw);

int mv_pp2x_cls_c2_sw_dump(struct mv_pp2x_cls_c2_entry *c2);
int mv_pp2x_cls_c2_hw_dump(struct mv_pp2x_hw *hw);
int mv_pp2x_cls_c2_qos_dscp_hw_dump(struct mv_pp2x_hw *hw);
int mv_pp2x_cls_c2_qos_prio_hw_dump(struct mv_pp2x_hw *hw);

void mv_pp2x_pp2_basic_print(struct platform_device *pdev,
			     struct mv_pp2x *priv);
void mv_pp2x_pp2_port_print(struct mv_pp2x_port *port);
void mv_pp2x_pp2_ports_print(struct mv_pp2x *priv);

int mv_pp2x_cls_hw_lkp_print(struct mv_pp2x_hw *hw, int lkpid, int way);
int mv_pp2x_cls_sw_flow_dump(struct mv_pp2x_cls_flow_entry *fe);
int mv_pp2x_cls_hw_regs_dump(struct mv_pp2x_hw *hw);
int mv_pp2x_cls_hw_flow_dump(struct mv_pp2x_hw *hw);
int mv_pp2x_cls_hw_flow_hits_dump(struct mv_pp2x_hw *hw);
int mv_pp2x_cls_hw_lkp_hits_dump(struct mv_pp2x_hw *hw);
int mv_pp2x_cls_sw_lkp_dump(struct mv_pp2x_cls_lookup_entry *lkp);
int mv_pp2x_cls_hw_lkp_dump(struct mv_pp2x_hw *hw);

int mv_pp2x_cls_c2_sw_words_dump(struct mv_pp2x_cls_c2_entry *c2);
int mv_pp2x_cls_c2_hit_cntr_dump(struct mv_pp2x_hw *hw);
int mv_pp2x_cls_c2_regs_dump(struct mv_pp2x_hw *hw);
int mv_pp22_rss_hw_dump(struct mv_pp2x_hw *hw);
int mv_pp22_rss_hw_rxq_tbl_dump(struct mv_pp2x_hw *hw);

int mvPp2RateCalc(int rate, unsigned int accuracy, unsigned int *pPeriod,
		  unsigned int *pTokens);

#endif /* _MVPP2_SYSFS_DEBUG_H_ */

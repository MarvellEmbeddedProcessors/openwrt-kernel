#
# Makefile for the Marvell Gigabit Ethernet driver
# NOTE: If compile driver for pp2x soc test, please run like "make TARGET=SOC_TEST"
#

M:=$(CURDIR)
KDIR:=$(srctree)

#$(info Makefile_sys M = $(M))

ifndef KDIR
$(error KDIR is not set. must declare kernel directory. example: export KDIR=~/linux-4.0.4/kernel/)
endif


obj-m := mvpp2x_sysfs.o
#obj-m += /mvebu_net
#obj-$m += mv_pp2.o

SYSFS_MOD_DIR = $(M)
ifdef LOCAL_SOURCES
PP2_MOD_DIR = $(SYSFS_MOD_DIR)/..
else
PP2_MOD_DIR = $(KDIR)/drivers/net/ethernet/marvell/mvpp2x
endif

MV_EBU_NET_DIR = $(SYSFS_MOD_DIR)/mvebu_net
MACH_MVEBU_DIR = $(SYSFS_MOD_DIR)/mach-mvebu

#export MACH_MVEBU_DIR
ccflags-y := -I$(SYSFS_MOD_DIR)
ccflags-y += -I${PP2_MOD_DIR}
#ccflags-y += -DMVPP21
#ccflags-y += -I${MV_EBU_NET_DIR}
#ccflags-y += -I${MACH_MVEBU_DIR}

ccflags-y += -I${KDIR}/include

mvpp2x_sysfs-y := mv_pp2x_sysfs_main.o mv_pp2x_sysfs_prs_high.o mv_pp2x_sysfs_prs_low.o mv_pp2x_sysfs_cls.o mv_pp2x_sysfs_cls2.o
mvpp2x_sysfs-y += mv_pp2x_sysfs_eth_bm.o mv_pp2x_sysfs_eth_rx.o mv_pp2x_sysfs_eth_tx.o mv_pp2x_sysfs_rss.o mv_pp2x_sysfs_cos.o mv_pp2x_sysfs_debug.o mv_pp2x_sysfs_debug_dump.o mv_pp2x_sysfs_debug_func.o
mvpp2x_sysfs-y += mv_pp2x_sysfs_eth_tx_sched.o mv_pp2x_sysfs_eth.o mv_pp2x_sysfs_eth_gop.o mv_pp2x_sysfs_eth_fca.o mv_pp2x_sysfs_musdk.o
ifeq (SOC_TEST,$(TARGET))
mvpp2x_sysfs-y += mv_pp2x_sysfs_cls3.o mv_pp2x_sysfs_cls4.o mv_pp2x_sysfs_pme.o mv_pp2x_sysfs_cls_mc.o mv_pp2x_sysfs_plcr.o
ccflags-y += -DMVPP2_SOC_TEST
endif

#PLAT_DIR 	:= $(MV_EBU_NET_DIR)
#export PLAT_DIR

#include $(PLAT_DIR)/Makefile
#include $(PLAT_DIR)/pp2/Makefile


####From mvebu_net
CPU_ARCH    = ARM
ifeq ($(CONFIG_CPU_BIG_ENDIAN),y)
ENDIAN      = BE
else
ENDIAN      = LE
endif

MVEBU_NET_FLAGS := -DMV_LINUX -DMV_CPU_$(ENDIAN) -DMV_$(CPU_ARCH)
export MVEBU_NET_FLAGS

ccflags-y       += $(MVEBU_NET_FLAGS)


#INCLUDE_DIRS	+= -I$(PLAT_DIR)
#INCLUDE_DIRS	+= -I$(PLAT_DIR)/common
#INCLUDE_DIRS	+= -I$(PLAT_DIR)/phy

#INCLUDE_DIRS	+= -I$(MACH_MVEBU_DIR)
#INCLUDE_DIRS	+= -I$(MACH_MVEBU_DIR)/linux_oss
#INCLUDE_DIRS	+= -I$(MACH_MVEBU_DIR)/include


#ccflags-y	+= $(INCLUDE_DIRS)

#export INCLUDE_DIRS



all:
	echo 'PP2_MOD_DIR=$(PP2_MOD_DIR)'
	make -C ${KDIR} M=$(M) modules

clean:
	make -C ${KDIR} SUBDIRS=$(M) clean


#$(PLAT_DIR)/pp2/mv_pp2.o :
#$(PLAT_DIR)/pp2/prs/prs_low_sysfs.o :
#	echo 'ERROR: mv_pp2.o is assumed to be compiled separately, beforehand.'
#$(PLAT_DIR)/pp2/prs/prs_high_sysfs.o :
#	echo 'ERROR: mv_pp2.o is assumed to be compiled separately, beforehand.'

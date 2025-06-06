/* SPDX-License-Identifier: GPL-2.0-only */
/*
################################################################################
#
# r8125 is the Linux device driver released for Realtek 2.5 Gigabit Ethernet
# controllers with PCI-Express interface.
#
# Copyright(c) 2025 Realtek Semiconductor Corp. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, see <http://www.gnu.org/licenses/>.
#
# Author:
# Realtek NIC software team <nicfae@realtek.com>
# No. 2, Innovation Road II, Hsinchu Science Park, Hsinchu 300, Taiwan
#
################################################################################
*/

/************************************************************************************
 *  This product is covered by one or more of the following patents:
 *  US6,570,884, US6,115,776, and US6,327,625.
 ***********************************************************************************/

#ifndef _LINUX_R8125_DASH_H
#define _LINUX_R8125_DASH_H

#include <linux/if.h>

#define SIOCDEVPRIVATE_RTLDASH   SIOCDEVPRIVATE+2

enum rtl_dash_cmd {
        RTL_DASH_ARP_NS_OFFLOAD = 0,
        RTL_DASH_SET_OOB_IPMAC,
        RTL_DASH_NOTIFY_OOB,

        RTL_DASH_SEND_BUFFER_DATA_TO_DASH_FW,
        RTL_DASH_CHECK_SEND_BUFFER_TO_DASH_FW_COMPLETE,
        RTL_DASH_GET_RCV_FROM_FW_BUFFER_DATA,
        RTL_DASH_OOB_REQ,
        RTL_DASH_OOB_ACK,
        RTL_DASH_DETACH_OOB_REQ,
        RTL_DASH_DETACH_OOB_ACK,

        RTL_FW_SET_IPV4 = 0x10,
        RTL_FW_GET_IPV4,
        RTL_FW_SET_IPV6,
        RTL_FW_GET_IPV6,
        RTL_FW_SET_EXT_SNMP,
        RTL_FW_GET_EXT_SNMP,
        RTL_FW_SET_WAKEUP_PATTERN,
        RTL_FW_GET_WAKEUP_PATTERN,
        RTL_FW_DEL_WAKEUP_PATTERN,

        RTLT_DASH_COMMAND_INVALID,
};

struct rtl_dash_ip_mac {
        struct sockaddr ifru_addr;
        struct sockaddr ifru_netmask;
        struct sockaddr ifru_hwaddr;
};

struct rtl_dash_ioctl_struct {
        __u32	cmd;
        __u32	offset;
        __u32	len;
        union {
                __u32	data;
                void *data_buffer;
        };
};

typedef struct _OSOOBHdr {
        __le32 len;
        u8 type;
        u8 flag;
        u8 hostReqV;
        u8 res;
}
OSOOBHdr, *POSOOBHdr;

typedef struct _RX_DASH_BUFFER_TYPE_2 {
        OSOOBHdr oobhdr;
        u8 RxDataBuffer[0];
}
RX_DASH_BUFFER_TYPE_2, *PRX_DASH_BUFFER_TYPE_2;

#define ALIGN_8                 (0x7)
#define ALIGN_16                (0xf)
#define ALIGN_32                (0x1f)
#define ALIGN_64                (0x3f)
#define ALIGN_256               (0xff)
#define ALIGN_4096              (0xfff)

#define OCP_REG_FIRMWARE_MAJOR_VERSION (0x120)

#define HW_DASH_SUPPORT_DASH(_M)        ((_M)->HwSuppDashVer > 0)
#define HW_DASH_SUPPORT_TYPE_1(_M)      ((_M)->HwSuppDashVer == 1)
#define HW_DASH_SUPPORT_TYPE_2(_M)      ((_M)->HwSuppDashVer == 2)
#define HW_DASH_SUPPORT_TYPE_3(_M)      ((_M)->HwSuppDashVer == 3)
#define HW_DASH_SUPPORT_TYPE_4(_M)      ((_M)->HwSuppDashVer == 4)
#define HW_DASH_SUPPORT_CMAC(_M)        (HW_DASH_SUPPORT_TYPE_2(_M) || HW_DASH_SUPPORT_TYPE_3(_M))
#define HW_DASH_SUPPORT_IPC2(_M)        (HW_DASH_SUPPORT_TYPE_4(_M))
#define HW_DASH_SUPPORT_GET_FIRMWARE_VERSION(_M) (HW_DASH_SUPPORT_TYPE_2(_M) || \
                                                  HW_DASH_SUPPORT_TYPE_3(_M) || \
                                                  HW_DASH_SUPPORT_TYPE_4(_M))

#define RECV_FROM_FW_BUF_SIZE (1520)
#define SEND_TO_FW_BUF_SIZE (1520)

#define TXS_CC3_0       (BIT_0|BIT_1|BIT_2|BIT_3)
#define TXS_EXC         BIT_4
#define TXS_LNKF        BIT_5
#define TXS_OWC         BIT_6
#define TXS_TES         BIT_7
#define TXS_UNF         BIT_9
#define TXS_LGSEN       BIT_11
#define TXS_LS          BIT_12
#define TXS_FS          BIT_13
#define TXS_EOR         BIT_14
#define TXS_OWN         BIT_15

#define TPPool_HRDY     0x20

#define RXS_OWN      BIT_15
#define RXS_EOR      BIT_14
#define RXS_FS       BIT_13
#define RXS_LS       BIT_12

#define ISRIMR_DASH_INTR_EN BIT_12

#define NO_BASE_ADDRESS 0x00000000

/* IB2SOC registers */
#define IPC2_SWISR_DRIVER_READY 0x05
#define IPC2_SWISR_DRIVER_EXIT 0x06
#define IPC2_SWISR_CLIENTTOOL_SYNC_HOSTNAME 0x20
#define IPC2_SWISR_DIS_DASH 0x55
#define IPC2_SWISR_EN_DASH 0x56

#define IPC2_IB2SOC_SET 0x10
#define IPC2_IB2SOC_DATA 0x14
#define IPC2_IB2SOC_CMD 0x18
#define IPC2_IB2SOC_IMR 0x1C

/* IPC2 registers */
#define IPC2_PCIE_BASE      0xC100
#define IPC2_TX_SET_REG     IPC2_PCIE_BASE
#define IPC2_TX_STATUS_REG  (IPC2_PCIE_BASE+0x04)
#define IPC2_RX_STATUS_REG  (IPC2_PCIE_BASE+0x08)
#define IPC2_RX_CLEAR_REG   (IPC2_PCIE_BASE+0x0C)
#define IPC2_DATA_BASE      0x32000
#define IPC2_BUFFER_LENGTH  0x1000
#define IPC2_DATA_MASTER    IPC2_DATA_BASE                        //dash tx buffer base
#define IPC2_DATA_SLAVE     (IPC2_DATA_BASE+IPC2_BUFFER_LENGTH)   //dash rx buffer base
#define IPC2_TX_BUFFER      IPC2_DATA_MASTER
#define IPC2_RX_BUFFER      IPC2_DATA_SLAVE

#define IPC2_TX_SEND_BIT        BIT_0
#define IPC2_TX_ACK_BIT         BIT_8
#define IPC2_RX_ROK_BIT         BIT_0
#define IPC2_RX_ACK_BIT         BIT_8

/* IPC2 write/read MMIO register */
#define RTL_DASH_IPC2_W8(tp, reg, val8)   RTL_W8(tp, reg, val8)
#define RTL_DASH_IPC2_W16(tp, reg, val16) RTL_W16(tp, reg, val16)
#define RTL_DASH_IPC2_W32(tp, reg, val32)  RTL_W32(tp, reg, val32)
#define RTL_DASH_IPC2_R8(tp, reg)     RTL_R8(tp, reg)
#define RTL_DASH_IPC2_R16(tp, reg)    RTL_R16(tp, reg)
#define RTL_DASH_IPC2_R32(tp, reg)    RTL_R32(tp, reg)

/* DASH OOB Header Type */
#define DASH_OOB_HDR_TYPE_REQ 0x91
#define DASH_OOB_HDR_TYPE_ACK 0x92

struct  rtl8125_private;

int rtl8125_dash_ioctl(struct net_device *dev, struct ifreq *ifr);
bool rtl8125_check_dash_interrupt(struct rtl8125_private *tp);
void rtl8125_handle_dash_interrupt(struct net_device *dev);
void rtl8125_clear_ipc2_isr(struct rtl8125_private *tp);
void rtl8125_set_ipc2_soc_imr_bit(struct rtl8125_private *tp, u16 mask);
void rtl8125_clear_ipc2_soc_imr_bit(struct rtl8125_private *tp, u16 mask);

#endif /* _LINUX_R8125_DASH_H */

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

#ifndef _LINUX_R8125_FIBER_H
#define _LINUX_R8125_FIBER_H

enum {
        FIBER_MODE_NIC_ONLY = 0,
        FIBER_MODE_RTL8125D_RTL8221D,
        FIBER_MODE_MAX
};

enum {
        FIBER_STAT_NOT_CHECKED = 0,
        FIBER_STAT_DISCONNECT,
        FIBER_STAT_CONNECT_GPO_C45,
        FIBER_STAT_MAX
};

#define HW_FIBER_MODE_ENABLED(_M)        ((_M)->HwFiberModeVer > 0)
#define HW_FIBER_STATUS_CONNECTED(_M)        (((_M)->HwFiberStat == FIBER_STAT_CONNECT_GPO_C45))
#define HW_FIBER_STATUS_DISCONNECTED(_M)        ((_M)->HwFiberStat == FIBER_STAT_DISCONNECT)

struct rtl8125_private;

void rtl8125_hw_fiber_phy_config(struct rtl8125_private *tp);
void rtl8125_check_fiber_mode_support(struct rtl8125_private *tp);
void rtl8125_fiber_mdio_write( struct rtl8125_private *tp, u32 reg, u16 val);
u16 rtl8125_fiber_mdio_read(struct rtl8125_private *tp, u32 reg);
unsigned int rtl8125_fiber_link_ok(struct net_device *dev);

#endif /* _LINUX_R8125_FIBER_H */

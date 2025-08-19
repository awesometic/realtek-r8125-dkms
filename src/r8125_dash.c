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

#include <linux/module.h>
#include <linux/version.h>
#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/if_vlan.h>
#include <linux/crc32.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/init.h>
#include <linux/rtnetlink.h>
#include <linux/completion.h>

#include <asm/uaccess.h>

#include "r8125.h"
#include "r8125_dash.h"
#include "rtl_eeprom.h"

static void r8125_dash_set_ipc2_reg_bit(struct rtl8125_private *tp, unsigned long reg, u32 mask)
{
        RTL_DASH_IPC2_W32(tp, reg, RTL_DASH_IPC2_R32(tp, reg) | mask);
}

/*
static void r8125_dash_clear_ipc2_reg_bit(struct rtl8125_private *tp, unsigned long reg, u32 mask)
{
        RTL_DASH_IPC2_W32(tp, reg, RTL_DASH_IPC2_R32(tp, reg) & ~mask);
}
*/

static void r8125_write_ipc2_tx_ack(struct rtl8125_private *tp)
{
        if (!tp->DASH)
                return;

        if (!HW_DASH_SUPPORT_IPC2(tp))
                return;

        r8125_dash_set_ipc2_reg_bit(tp, IPC2_TX_SET_REG, IPC2_TX_ACK_BIT);
}

static void r8125_write_ipc2_tx_polling(struct rtl8125_private *tp)
{
        if (!tp->DASH)
                return;

        if (!HW_DASH_SUPPORT_IPC2(tp))
                return;

        r8125_dash_set_ipc2_reg_bit(tp, IPC2_TX_SET_REG, IPC2_TX_SEND_BIT);
}

static unsigned long
r8125_get_ipc2_rx_buffer(struct rtl8125_private *tp)
{
        if (HW_DASH_SUPPORT_IPC2(tp))
                return IPC2_RX_BUFFER;
        else
                return 0;
}

static u8 rtl8125_copy_from_ipc2(struct rtl8125_private *tp, u8 *dest, u32 len)
{
        unsigned long const data_reg = r8125_get_ipc2_rx_buffer(tp);
        u32 offset = 0;
        u32 *pDword;
        u8 *pByte;

        if (FALSE == HW_DASH_SUPPORT_IPC2(tp))
                goto exit;

        if (!dest)
                goto exit;

        if (len == 0)
                goto exit;

        pDword = (u32*)dest;
        while (len > 3 && offset < (IPC2_BUFFER_LENGTH - 4)) {
                *pDword++ = RTL_DASH_IPC2_R32(tp, data_reg + offset);

                len -= 4;
                offset += 4;
        }

        pByte = (u8*)pDword;
        while (len > 0 && offset < (IPC2_BUFFER_LENGTH - 1)) {
                *pByte++ = RTL_DASH_IPC2_R8(tp, data_reg + offset);

                len -= 1;
                offset += 1;
        }

exit:
        return (len == 0) ? TRUE : FALSE;
}

static void RecvFromDashFwComplete(struct rtl8125_private *tp)
{
        if (!tp->DASH)
                return;

        if (!HW_DASH_SUPPORT_IPC2(tp))
                return;

        if (tp->DashReqRegValue == DASH_OOB_HDR_TYPE_REQ) { //rok
                RX_DASH_BUFFER_TYPE_2 rxDashBufferType2 = {0};
                u32 dataLen;

                if (!tp->OobReq)
                        goto exit;

                /* copy header for check data length */
                if (!rtl8125_copy_from_ipc2(tp,
                                            (u8*)&rxDashBufferType2,
                                            sizeof(rxDashBufferType2)))
                        goto exit;

                dataLen = (u16)rxDashBufferType2.oobhdr.len;

                tp->AfterRecvFromFwBufLen = dataLen + sizeof(OSOOBHdr);
                if (tp->AfterRecvFromFwBufLen > tp->SizeOfRecvFromFwBuffer) {
                        tp->AfterRecvFromFwBufLen = tp->SizeOfRecvFromFwBuffer;
                        tp->RecvFromFwBufErrCnt++;
                }

                /* copy data */
                rtl8125_copy_from_ipc2(tp,
                                       tp->AfterRecvFromFwBuf,
                                       tp->AfterRecvFromFwBufLen);

                r8125_write_ipc2_tx_ack(tp);

                tp->OobReqComplete = TRUE;

                tp->RecvFromDashFwCnt++;
        } else if (tp->DashReqRegValue == DASH_OOB_HDR_TYPE_ACK) { //rx ack
                if (!tp->OobAck)
                        goto exit;

                tp->OobAckComplete = TRUE;

                tp->RecvFromDashFwCnt++;
        }

exit:
        return;
}

static unsigned long r8125_get_ipc2_tx_buffer(struct rtl8125_private *tp)
{
        if (HW_DASH_SUPPORT_IPC2(tp))
                return IPC2_TX_BUFFER;
        else
                return 0;
}

static u32 rtl8125_copy_to_ipc2(struct rtl8125_private *tp, u8 *src, u32 len)
{
        unsigned long const data_reg = r8125_get_ipc2_tx_buffer(tp);
        u32 offset = 0;
        u32 *pDword;
        u8 *pByte;

        if (FALSE == HW_DASH_SUPPORT_IPC2(tp))
                goto exit;

        if (!src)
                goto exit;

        if (len == 0)
                goto exit;

        pDword = (u32*)src;
        while (len > 3 && offset < (IPC2_BUFFER_LENGTH - 4)) {
                RTL_DASH_IPC2_W32(tp, data_reg + offset, *pDword++);

                len -= 4;
                offset += 4;
        }

        pByte = (u8*)pDword;
        while (len > 0 && offset < (IPC2_BUFFER_LENGTH - 1)) {
                RTL_DASH_IPC2_W8(tp, data_reg + offset, *pByte++);

                len -= 1;
                offset += 1;
        }

exit:
        return offset;
}

static int SendToDashFw(struct rtl8125_private *tp, u8 *src, u16 len)
{
        POSOOBHdr pOobHdr;
        int rc = -1;

        if (!tp->DASH)
                goto exit;

        if (FALSE == HW_DASH_SUPPORT_IPC2(tp))
                goto exit;

        if (TRUE == tp->SendingToFw)
                goto exit;

        if (!src)
                goto exit;

        if (len > tp->SizeOfSendToFwBuffer)
                goto exit;

        if (len < sizeof(OSOOBHdr))
                goto exit;

        pOobHdr = (POSOOBHdr)src;
        if (pOobHdr->hostReqV == DASH_OOB_HDR_TYPE_REQ) {
                r8125_write_ipc2_tx_ack(tp);
                rc = 0;
                goto exit;
        }

        tp->SendingToFw = TRUE;

        rtl8125_copy_to_ipc2(tp, src, len);

        r8125_write_ipc2_tx_polling(tp);

        tp->SendingToFw = FALSE;

        rc = 0;

exit:
        if (!rc)
                tp->AfterSendToFwBufLen = len;
        else
                tp->AfterSendToFwBufLen = 0;

        return rc;
}

static u32 rtl8125_get_ipc2_isr(struct rtl8125_private *tp)
{
        u32 isr = 0;

        if (FALSE == HW_DASH_SUPPORT_IPC2(tp))
                goto exit;

        isr = RTL_DASH_IPC2_R32(tp, IPC2_RX_STATUS_REG);

        if (isr == ULONG_MAX)
                isr = 0;

exit:
        return isr;
}

static void rtl8125_set_ipc2_isr(struct rtl8125_private *tp, u32 val)
{
        if (FALSE == HW_DASH_SUPPORT_IPC2(tp))
                return;

        RTL_DASH_IPC2_W32(tp, IPC2_RX_CLEAR_REG, val);
}

void rtl8125_clear_ipc2_isr(struct rtl8125_private *tp)
{
        rtl8125_set_ipc2_isr(tp, rtl8125_get_ipc2_isr(tp));
}

void rtl8125_set_ipc2_soc_imr_bit(struct rtl8125_private *tp, u16 mask)
{
        if (FALSE == HW_DASH_SUPPORT_IPC2(tp))
                return;

        RTL_W16(tp, RISC_IMR_8125BP, RTL_R16(tp, RISC_IMR_8125BP) | mask);
}

void rtl8125_clear_ipc2_soc_imr_bit(struct rtl8125_private *tp, u16 mask)
{
        if (FALSE == HW_DASH_SUPPORT_IPC2(tp))
                return;

        RTL_W16(tp, RISC_IMR_8125BP, RTL_R16(tp, RISC_IMR_8125BP) & ~mask);
}

bool rtl8125_check_dash_interrupt(struct rtl8125_private *tp)
{
        bool rc = false;
        u32 isr;

        if(!tp->DASH)
                goto exit;

        if (FALSE == HW_DASH_SUPPORT_IPC2(tp))
                goto exit;

        isr = rtl8125_get_ipc2_isr(tp);

        if (isr & (IPC2_RX_ROK_BIT | IPC2_RX_ACK_BIT)) {
                set_bit(R8125_RCV_REQ_DASH_OK, tp->dash_req_flags);
                if (isr & IPC2_RX_ROK_BIT)
                        tp->DashReqRegValue = DASH_OOB_HDR_TYPE_REQ;
                else
                        tp->DashReqRegValue = DASH_OOB_HDR_TYPE_ACK;
        }

        rtl8125_set_ipc2_isr(tp, isr);

exit:
        return rc;
}

void rtl8125_handle_dash_interrupt(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        if(!tp->DASH)
                return;

        if (test_and_clear_bit(R8125_RCV_REQ_DASH_OK, tp->dash_req_flags))
                RecvFromDashFwComplete(tp);
}

static int DashIoctlGetRcvFromFwData(struct net_device *dev, struct rtl_dash_ioctl_struct *prtl_dash_usrdata)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        u32 ulInfoLen;
        void *InformationBuffer;
        u32 InformationBufferLength;
        void *pInfo;
        u8 *pByte;
        u16 *pWord;
        u8 *tmpBuf;
        int ret = -EFAULT;

        if (!tp->DASH)
                goto exit;

        if (!tp->rtk_enable_diag)
                goto exit;

        if (tp->AfterRecvFromFwBufLen == 0)
                goto exit;

        InformationBufferLength = prtl_dash_usrdata->len;
        InformationBuffer = prtl_dash_usrdata->data_buffer;

        ulInfoLen = tp->AfterRecvFromFwBufLen + 2 + 2;
        if (InformationBufferLength < ulInfoLen) {
                ret = -EFAULT;
                goto exit;
        }

        if (!(tmpBuf = kmalloc(ulInfoLen, GFP_ATOMIC))) {
                ret = -ENOMEM;
                goto exit;
        }

        pInfo = (void*) tp->AfterRecvFromFwBuf;
        pWord = (u16*) tmpBuf;
        *pWord++ = tp->AfterRecvFromFwBufLen;
        pByte = (u8*)pWord;
        memcpy(pByte, pInfo, tp->AfterRecvFromFwBufLen);
        pWord = (u16*)(pByte + tp->AfterRecvFromFwBufLen);
        *pWord= tp->DashReqRegValue;
        tp->AfterRecvFromFwBufLen = 0;
        if (copy_to_user(InformationBuffer, tmpBuf, ulInfoLen)) {
                kfree(tmpBuf);
                ret = -EFAULT;
                goto exit;
        }
        kfree(tmpBuf);
        ret = 0;

exit:
        return ret;
}

static int DashIoctlCheckSendBufferToFwComplete(struct net_device *dev, struct rtl_dash_ioctl_struct *prtl_dash_usrdata)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        u32 ulInfoLen;
        void *InformationBuffer;
        u32 InformationBufferLength;
        u16 *pWord;
        u8 *tmpBuf;
        int ret = -EFAULT;

        if (!tp->DASH)
                goto exit;

        if (!tp->rtk_enable_diag)
                goto exit;

        InformationBufferLength = prtl_dash_usrdata->len;
        InformationBuffer = prtl_dash_usrdata->data_buffer;

        if (tp->SendingToFw == FALSE)
                ulInfoLen = tp->AfterSendToFwBufLen + sizeof(u16);
        else
                ulInfoLen = sizeof(u16);

        if (InformationBufferLength < ulInfoLen) {
                ret = -EFAULT;
                goto exit;
        }

        if (!(tmpBuf = kmalloc(ulInfoLen, GFP_ATOMIC))) {
                ret = -ENOMEM;
                goto exit;
        }

        pWord = (u16*) tmpBuf;
        if (tp->SendingToFw == FALSE) {
                *pWord++ = tp->AfterSendToFwBufLen;
                memcpy(pWord, tp->AfterSendToFwBuf, tp->AfterSendToFwBufLen);
                tp->AfterSendToFwBufLen = 0;
        } else {
                *pWord = 0xffff;
        }

        if (copy_to_user(InformationBuffer, tmpBuf, ulInfoLen))
                ret = -EFAULT;
        else
                ret = 0;

        kfree(tmpBuf);

exit:
        return ret;
}

static int DashIoctlCheckSendBufferToFw(struct net_device *dev, struct rtl_dash_ioctl_struct *prtl_dash_usrdata)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        u32 ulInfoLen;
        void *InformationBuffer;
        u32 InformationBufferLength;
        u16 *pWord;
        u16 SetDataSize;
        int ret = -EFAULT;

        if (!tp->DASH)
                goto exit;

        if (!tp->rtk_enable_diag)
                goto exit;

        InformationBufferLength = prtl_dash_usrdata->len;
        if (!(InformationBuffer = kmalloc(InformationBufferLength, GFP_KERNEL))) {
                ret = -ENOMEM;
                goto exit;
        }

        if (copy_from_user(InformationBuffer, prtl_dash_usrdata->data_buffer,
                           InformationBufferLength)) {
                ret = -EFAULT;
                goto free_mem;
        }

        ulInfoLen = sizeof(u16) + sizeof(u16);

        if (InformationBufferLength < ulInfoLen)
                goto free_mem;

        pWord = (u16*) InformationBuffer;
        SetDataSize = *pWord++;

        if (InformationBufferLength < (SetDataSize + sizeof(u16) + sizeof(u16))) {
                ret = -EFAULT;
                goto free_mem;
        }

        ret = SendToDashFw(tp, (u8*)pWord, SetDataSize);

free_mem:
        kfree(InformationBuffer);

exit:
        return ret;
}

int rtl8125_dash_ioctl(struct net_device *dev, struct ifreq *ifr)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        void *user_data = ifr->ifr_data;
        struct rtl_dash_ioctl_struct rtl_dash_usrdata;

        int ret=0;

        if (FALSE == HW_DASH_SUPPORT_DASH(tp))
                return -EOPNOTSUPP;

        if (!tp->DASH)
                return -EINVAL;

        if (copy_from_user(&rtl_dash_usrdata, user_data,
                           sizeof(struct rtl_dash_ioctl_struct)))
                return -EFAULT;

        switch (rtl_dash_usrdata.cmd) {
        case RTL_DASH_SEND_BUFFER_DATA_TO_DASH_FW:
                ret = DashIoctlCheckSendBufferToFw(dev, &rtl_dash_usrdata);
                break;
        case RTL_DASH_CHECK_SEND_BUFFER_TO_DASH_FW_COMPLETE:
                ret = DashIoctlCheckSendBufferToFwComplete(dev,
                                &rtl_dash_usrdata);
                break;
        case RTL_DASH_GET_RCV_FROM_FW_BUFFER_DATA:
                ret = DashIoctlGetRcvFromFwData(dev, &rtl_dash_usrdata);
                break;
        case RTL_DASH_OOB_REQ:
                tp->OobReq = TRUE;
                tp->OobReqComplete = FALSE;
                break;
        case RTL_DASH_OOB_ACK:
                tp->OobAck = TRUE;
                tp->OobAckComplete = FALSE;
                break;
        case RTL_DASH_DETACH_OOB_REQ:
                tp->OobReq = FALSE;
                tp->OobReqComplete = FALSE;
                break;
        case RTL_DASH_DETACH_OOB_ACK:
                tp->OobAck = FALSE;
                tp->OobAckComplete = FALSE;
                break;
        default:
                return -EOPNOTSUPP;
        }

        return ret;
}

/*
 *  Copyright 2007 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

/*
 * mxc custom ioremap implementation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/hardware.h>
#include <asm/io.h>

void *__iomem __mxc_ioremap(unsigned long cookie, size_t size,
			    unsigned int mtype)
{
	if (mtype == MT_DEVICE && IS_MEM_DEVICE_NONSHARED(cookie)) {
		mtype = MT_DEVICE_NONSHARED;
	}
	return __arm_ioremap(cookie, size, mtype);
}

EXPORT_SYMBOL(__mxc_ioremap);

void __mxc_iounmap(void __iomem * addr)
{
	extern void __iounmap(volatile void __iomem * addr);

	__iounmap(addr);
}

EXPORT_SYMBOL(__mxc_iounmap);

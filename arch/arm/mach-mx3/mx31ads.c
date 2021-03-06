/*
 *  Copyright (C) 2000 Deep Blue Solutions Ltd
 *  Copyright (C) 2002 Shane Nay (shane@minirl.com)
 *  Copyright 2005-2007 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/types.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/serial_8250.h>
#ifdef CONFIG_KGDB_8250
#include <linux/kgdb.h>
#endif
#include <linux/input.h>
#include <linux/nodemask.h>
#include <linux/clk.h>
#include <linux/spi/spi.h>
#if defined(CONFIG_MTD) || defined(CONFIG_MTD_MODULE)
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>

#include <asm/mach/flash.h>
#endif

#include <asm/hardware.h>
#include <asm/irq.h>
#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/irq.h>
#include <asm/mach/keypad.h>
#include <asm/arch/memory.h>
#include <asm/arch/gpio.h>

#ifdef CONFIG_MACH_LAB126
#include <asm/arch-mxc/board-mx31ads.h>
#include <asm/arch/boot_globals.h>
#include <linux/bootmem.h>
#endif

#include "crm_regs.h"
#include "iomux.h"
/*!
 * @file mach-mx3/mx31ads.c
 *
 * @brief This file contains the board specific initialization routines.
 *
 * @ingroup MSL_MX31
 */

extern void mxc_map_io(void);
extern void mxc_init_irq(void);
extern void mxc_cpu_init(void) __init;
extern void mx31ads_gpio_init(void) __init;
extern struct sys_timer mxc_timer;
extern void mxc_cpu_common_init(void);
extern int mxc_clocks_init(void);
extern void __init early_console_setup(char *);

static void mxc_nop_release(struct device *dev)
{
	/* Nothing */
}

unsigned long board_get_ckih_rate(void)
{
	if ((__raw_readw(PBC_BASE_ADDRESS + PBC_BSTAT) &
	     CKIH_27MHZ_BIT_SET) != 0) {
		return 27000000;
	}
	return 26000000;
}

#if defined(CONFIG_CS89x0) || defined(CONFIG_CS89x0_MODULE)
/*! Null terminated portlist used to probe for the CS8900A device on ISA Bus
 * Add 3 to reset the page window before probing (fixes eth probe when deployed
 * using nand_boot)
 */
unsigned int netcard_portlist[] = { CS8900A_BASE_ADDRESS + 3, 0 };

EXPORT_SYMBOL(netcard_portlist);
/*!
 * The CS8900A has 4 IRQ pins, which is software selectable, CS8900A interrupt
 * pin 0 is used for interrupt generation.
 */
unsigned int cs8900_irq_map[] = { CS8900AIRQ, 0, 0, 0 };

EXPORT_SYMBOL(cs8900_irq_map);
#endif

#if defined(CONFIG_KEYBOARD_MXC) || defined(CONFIG_KEYBOARD_MXC_MODULE)

#ifdef CONFIG_MACH_LAB126
/* We just send the 58 possible "keys" that are available on Turing/Nell.  To
 * compensate fore the keys that aren't available on the ADS keypad but are
 * available on Turing/Nell, we provide the following mapping:
 *
 *  TAB     -> SHIFT
 *  SYMBOL  -> ALT
 *  #       -> FONT
 *  *       -> SYM
 *  ON/OFF  -> /
 *  APP1    -> PREV
 *  APP2    -> NEXT (left)
 *  APP3    -> NEXT (right)
 *  APP4    -> MENU
 */
static u16 keymapping[64] = {
	KEY_HENKAN, KEY_LEFT, KEY_HANJA, KEY_RIGHT,
	KEY_HANGUEL, KEY_RESERVED, KEY_RESERVED, KEY_HIRAGANA,
	KEY_RESERVED, KEY_RESERVED, KEY_KPSLASH, KEY_PAGEDOWN,
	KEY_VOLUMEUP, KEY_PAGEUP, KEY_YEN, KEY_MENU,
	KEY_3, KEY_2, KEY_1, KEY_4,
	KEY_VOLUMEDOWN, KEY_7, KEY_5, KEY_6,
	KEY_9, KEY_KATAKANA, KEY_8, KEY_0,
	KEY_MUHENKAN, KEY_RESERVED, KEY_Q, KEY_W,
	KEY_A, KEY_S, KEY_D, KEY_E,
	KEY_F, KEY_R, KEY_T, KEY_Y,
	KEY_LEFTSHIFT, KEY_LEFTALT, KEY_RESERVED, KEY_Z,
	KEY_X, KEY_C, KEY_V, KEY_G,
	KEY_B, KEY_H, KEY_N, KEY_M,
	KEY_J, KEY_K, KEY_U, KEY_I,
	KEY_SPACE, KEY_SLASH, KEY_DOT, KEY_ENTER,
	KEY_L, KEY_BACKSPACE, KEY_P, KEY_O,
};
#else
/* Keypad keycodes for the EVB 8x8
 * keypad.  POWER and PTT keys don't generate
 * any interrupts via this driver so they are
 * not support. Change any keys as u like!
 */
static u16 keymapping[64] = {
	KEY_SELECT, KEY_LEFT, KEY_DOWN, KEY_RIGHT,
	KEY_UP, KEY_F12, KEY_END, KEY_BACK,
	KEY_F1, KEY_SENDFILE, KEY_HOME, KEY_F6,
	KEY_VOLUMEUP, KEY_F8, KEY_F9, KEY_F10,
	KEY_3, KEY_2, KEY_1, KEY_4,
	KEY_VOLUMEDOWN, KEY_7, KEY_5, KEY_6,
	KEY_9, KEY_LEFTSHIFT, KEY_8, KEY_0,
	KEY_KPASTERISK, KEY_RECORD, KEY_Q, KEY_W,
	KEY_A, KEY_S, KEY_D, KEY_E,
	KEY_F, KEY_R, KEY_T, KEY_Y,
	KEY_TAB, KEY_F7, KEY_CAPSLOCK, KEY_Z,
	KEY_X, KEY_C, KEY_V, KEY_G,
	KEY_B, KEY_H, KEY_N, KEY_M,
	KEY_J, KEY_K, KEY_U, KEY_I,
	KEY_SPACE, KEY_F2, KEY_DOT, KEY_ENTER,
	KEY_L, KEY_BACKSPACE, KEY_P, KEY_O,
};
#endif

static struct resource mxc_kpp_resources[] = {
	[0] = {
	       .start = INT_KPP,
	       .end = INT_KPP,
	       .flags = IORESOURCE_IRQ,
	       }
};

static struct keypad_data evb_8_by_8_keypad = {
	.rowmax = 8,
	.colmax = 8,
	.irq = INT_KPP,
	.learning = 0,
	.delay = 2,
	.matrix = keymapping,
};

/* mxc keypad driver */
static struct platform_device mxc_keypad_device = {
	.name = "mxc_keypad",
	.id = 0,
	.num_resources = ARRAY_SIZE(mxc_kpp_resources),
	.resource = mxc_kpp_resources,
	.dev = {
		.release = mxc_nop_release,
		.platform_data = &evb_8_by_8_keypad,
		},
};

static void mxc_init_keypad(void)
{
	(void)platform_device_register(&mxc_keypad_device);
}
#else
static inline void mxc_init_keypad(void)
{
}
#endif

#if defined(CONFIG_SERIAL_8250) || defined(CONFIG_SERIAL_8250_MODULE)
/*!
 * The serial port definition structure. The fields contain:
 * {UART, CLK, PORT, IRQ, FLAGS}
 */
static struct plat_serial8250_port serial_platform_data[] = {
	{
	 .membase = (void __iomem *)(PBC_BASE_ADDRESS + PBC_SC16C652_UARTA),
	 .mapbase = (unsigned long)(CS4_BASE_ADDR + PBC_SC16C652_UARTA),
	 .irq = EXPIO_INT_XUART_INTA,
	 .uartclk = 14745600,
	 .regshift = 0,
	 .iotype = UPIO_MEM,
	 .flags = UPF_BOOT_AUTOCONF | UPF_SKIP_TEST | UPF_AUTO_IRQ,
	 },
	{
	 .membase = (void __iomem *)(PBC_BASE_ADDRESS + PBC_SC16C652_UARTB),
	 .mapbase = (unsigned long)(CS4_BASE_ADDR + PBC_SC16C652_UARTB),
	 .irq = EXPIO_INT_XUART_INTB,
	 .uartclk = 14745600,
	 .regshift = 0,
	 .iotype = UPIO_MEM,
	 .flags = UPF_BOOT_AUTOCONF | UPF_SKIP_TEST | UPF_AUTO_IRQ,
	 },
	{},
};

/*!
 * REVISIT: document me
 */
static struct platform_device serial_device = {
	.name = "serial8250",
	.id = 0,
	.dev = {
		.platform_data = serial_platform_data,
		},
};

/*!
 * REVISIT: document me
 */
static int __init mxc_init_extuart(void)
{
	return platform_device_register(&serial_device);
}
#else
static inline int mxc_init_extuart(void)
{
	return 0;
}
#endif
/* MTD NOR flash */

#if defined(CONFIG_MTD_MXC) || defined(CONFIG_MTD_MXC_MODULE)

static struct mtd_partition mxc_nor_partitions[] = {
	{
	 .name = "Bootloader",
	 .size = 2 * 512 * 1024,
	 .offset = 0x00000000,
	 .mask_flags = MTD_WRITEABLE	/* force read-only */
	 },
	{
	 .name = "Kernel",
	 .size = 2 * 1024 * 1024,
	 .offset = MTDPART_OFS_APPEND,
	 .mask_flags = 0},
	{
	 .name = "userfs",
	 .size = 1 * 1024 * 1024,
	 .offset = MTDPART_OFS_APPEND,
	 .mask_flags = 0},
	{
	 .name = "rootfs",
	 .size = 3 * 1024 * 1024,
	 .offset = MTDPART_OFS_APPEND,
	 .mask_flags = MTD_WRITEABLE},
	{
	 .name = "FIS directory",
	 .size = 12 * 1024,
	 .offset = 0x007FF00-(12*1024), // 00x007FE000,
	 .mask_flags = MTD_WRITEABLE	/* force read-only */
	 },
	{
	 .name = "Redboot config",
	 .size = MTDPART_SIZ_FULL,
	 .offset = 0x007FF000,
	 .mask_flags = MTD_WRITEABLE	/* force read-only */
	 },
};

static struct flash_platform_data mxc_flash_data = {
	.map_name = "cfi_probe",
	.width = 2,
	.parts = mxc_nor_partitions,
	.nr_parts = ARRAY_SIZE(mxc_nor_partitions),
};

static struct resource mxc_flash_resource = {
	.start = 0xa0000000,
	.end = 0xa0000000 + 0x02000000 - 1,
	.flags = IORESOURCE_MEM,

};

static struct platform_device mxc_nor_mtd_device = {
	.name = "mxc_nor_flash",
	.id = 0,
	.dev = {
		.release = mxc_nop_release,
		.platform_data = &mxc_flash_data,
		},
	.num_resources = 1,
	.resource = &mxc_flash_resource,
};

static void mxc_init_nor_mtd(void)
{
	(void)platform_device_register(&mxc_nor_mtd_device);
}
#else
static void mxc_init_nor_mtd(void)
{
}
#endif

/* MTD NAND flash */

#if defined(CONFIG_MTD_NAND_MXC) || defined(CONFIG_MTD_NAND_MXC_MODULE) \
	|| defined(CONFIG_MTD_NAND_MXC_V2) || defined(CONFIG_MTD_NAND_MXC_V2_MODULE)

static struct mtd_partition mxc_nand_partitions[4] = {
	{
	 .name = "IPL-SPL",
	 .offset = 0,
	 .size = 256 * 1024},
	{
	 .name = "nand.kernel",
	 .offset = MTDPART_OFS_APPEND,
	 .size = 4 * 1024 * 1024},
	{
	 .name = "nand.rootfs",
	 .offset = MTDPART_OFS_APPEND,
	 .size = 22 * 1024 * 1024},
	{
	 .name = "nand.userfs",
	 .offset = MTDPART_OFS_APPEND,
	 .size = MTDPART_SIZ_FULL},
};

static struct flash_platform_data mxc_nand_data = {
	.parts = mxc_nand_partitions,
	.nr_parts = ARRAY_SIZE(mxc_nand_partitions),
	.width = 1,
};

static struct platform_device mxc_nand_mtd_device = {
	.name = "mxc_nand_flash",
	.id = 0,
	.dev = {
		.release = mxc_nop_release,
		.platform_data = &mxc_nand_data,
		},
};

static struct platform_device mxc_nandv2_mtd_device = {
	.name = "mxc_nandv2_flash",
	.id = 0,
	.dev = {
		.release = mxc_nop_release,
		.platform_data = &mxc_nand_data,
		},
};

static void mxc_init_nand_mtd(void)
{
	if (__raw_readl(MXC_CCM_RCSR) & MXC_CCM_RCSR_NF16B) {
		mxc_nand_data.width = 2;
	}
	if (cpu_is_mx31()) {
		(void)platform_device_register(&mxc_nand_mtd_device);
	}
	if (cpu_is_mx32()) {
		(void)platform_device_register(&mxc_nandv2_mtd_device);
	}
}
#else
static inline void mxc_init_nand_mtd(void)
{
}
#endif

static struct spi_board_info mxc_spi_board_info[] __initdata = {
	{
	 .modalias = "pmic_spi",
	 .irq = IOMUX_TO_IRQ(MX31_PIN_GPIO1_3),
	 .max_speed_hz = 4000000,
	 .bus_num = 2,
	 .chip_select = 0,
	 },
};

#if defined(CONFIG_FB_MXC_SYNC_PANEL) || defined(CONFIG_FB_MXC_SYNC_PANEL_MODULE)
static const char fb_default_mode[] = "Sharp-QVGA";

/* mxc lcd driver */
static struct platform_device mxc_fb_device = {
	.name = "mxc_sdc_fb",
	.id = 0,
	.dev = {
		.release = mxc_nop_release,
		.platform_data = &fb_default_mode,
		.coherent_dma_mask = 0xFFFFFFFF,
		},
};

static void mxc_init_fb(void)
{
	(void)platform_device_register(&mxc_fb_device);
}
#else
static inline void mxc_init_fb(void)
{
}
#endif

#if defined(CONFIG_MXC_FIR) || defined(CONFIG_MXC_FIR_MODULE)
/*!
 * Resource definition for the FIR
 */
static struct resource mxcir_resources[] = {
	[0] = {
	       .start = UART2_BASE_ADDR,
	       .end = UART2_BASE_ADDR + SZ_16K - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = INT_UART2,
	       .end = INT_UART2,
	       .flags = IORESOURCE_IRQ,
	       },
	[2] = {
	       .start = FIRI_BASE_ADDR,
	       .end = FIRI_BASE_ADDR + SZ_16K - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[3] = {
	       .start = INT_FIRI,
	       .end = INT_FIRI,
	       .flags = IORESOURCE_IRQ,
	       },
	[4] = {
	       .start = INT_UART2,
	       .end = INT_UART2,
	       .flags = IORESOURCE_IRQ,
	       }
};

static struct mxc_ir_platform_data ir_data = {
	.uart_ir_mux = 1,
	.ir_rx_invert = MXC_IRDA_RX_INV,
	.ir_tx_invert = MXC_IRDA_TX_INV,
};

/*! Device Definition for MXC FIR */
static struct platform_device mxcir_device = {
	.name = "mxcir",
	.id = 0,
	.dev = {
		.release = mxc_nop_release,
		.platform_data = &ir_data,
		},
	.num_resources = ARRAY_SIZE(mxcir_resources),
	.resource = mxcir_resources,
};

static inline void mxc_init_ir(void)
{
	ir_data.uart_clk = clk_get(NULL, "uart_clk.1");;
	(void)platform_device_register(&mxcir_device);
}
#else
static inline void mxc_init_ir(void)
{
}
#endif

static void mxc_expio_irq_handler(u32 irq, struct irq_desc *desc)
{
	u32 imr_val;
	u32 int_valid;
	u32 expio_irq;

	desc->chip->mask(irq);	/* irq = gpio irq number */

	imr_val = __raw_readw(PBC_INTMASK_SET_REG);
	int_valid = __raw_readw(PBC_INTSTATUS_REG) & imr_val;

	if (unlikely(!int_valid)) {
		printk(KERN_ERR "\nEXPIO: Spurious interrupt:0x%0x\n\n",
		       int_valid);
		goto out;
	}

	expio_irq = MXC_EXP_IO_BASE;
	for (; int_valid != 0; int_valid >>= 1, expio_irq++) {
		struct irq_desc *d;
		if ((int_valid & 1) == 0)
			continue;
		d = irq_desc + expio_irq;
		if (unlikely(!(d->handle_irq))) {
			printk(KERN_ERR "\nEXPIO irq: %d unhandeled\n",
			       expio_irq);
			BUG();	/* oops */
		}
		d->handle_irq(expio_irq, d);
	}

      out:
	desc->chip->ack(irq);
	desc->chip->unmask(irq);
}

/*
 * Disable an expio pin's interrupt by setting the bit in the imr.
 * @param irq		an expio virtual irq number
 */
static void expio_mask_irq(u32 irq)
{
	u32 expio = MXC_IRQ_TO_EXPIO(irq);
	/* mask the interrupt */
	__raw_writew(1 << expio, PBC_INTMASK_CLEAR_REG);
}

/*
 * Acknowledge an expanded io pin's interrupt by clearing the bit in the isr.
 * @param irq		an expanded io virtual irq number
 */
static void expio_ack_irq(u32 irq)
{
	u32 expio = MXC_IRQ_TO_EXPIO(irq);
	/* clear the interrupt status */
	__raw_writew(1 << expio, PBC_INTSTATUS_REG);
	/* mask the interrupt */
	expio_mask_irq(irq);
}

/*
 * Enable a expio pin's interrupt by clearing the bit in the imr.
 * @param irq		a expio virtual irq number
 */
static void expio_unmask_irq(u32 irq)
{
	u32 expio = MXC_IRQ_TO_EXPIO(irq);
	/* unmask the interrupt */
	__raw_writew(1 << expio, PBC_INTMASK_SET_REG);
}

static struct irq_chip expio_irq_chip = {
	.ack = expio_ack_irq,
	.mask = expio_mask_irq,
	.unmask = expio_unmask_irq,
};

static int initialized = 0;

static int __init _mxc_expio_init(void)
{
	int i;

	initialized = 1;

	printk(KERN_INFO "MX31ADS EXPIO(CPLD) hardware\n");
	/*
	 * Configure INT line as GPIO input
	 */
	mxc_request_iomux(MX31_PIN_GPIO1_4, OUTPUTCONFIG_GPIO,
			  INPUTCONFIG_GPIO);
	mxc_set_gpio_direction(MX31_PIN_GPIO1_4, 1);

	/* disable the interrupt and clear the status */
	__raw_writew(0xFFFF, PBC_INTMASK_CLEAR_REG);
	__raw_writew(0xFFFF, PBC_INTSTATUS_REG);
	for (i = MXC_EXP_IO_BASE; i < (MXC_EXP_IO_BASE + MXC_MAX_EXP_IO_LINES);
	     i++) {
		set_irq_chip(i, &expio_irq_chip);
		set_irq_handler(i, handle_level_irq);
		set_irq_flags(i, IRQF_VALID);
	}
	set_irq_type(EXPIO_PARENT_INT, IRQT_HIGH);
	set_irq_chained_handler(EXPIO_PARENT_INT, mxc_expio_irq_handler);

	return 0;
}

/*
 * This may get called early from board specific init
 */
int mxc_expio_init(void)
{
	if (!initialized)
		return _mxc_expio_init();
	else
		return 0;
}

/*!
 * Board specific fixup function. It is called by \b setup_arch() in
 * setup.c file very early on during kernel starts. It allows the user to
 * statically fill in the proper values for the passed-in parameters. None of
 * the parameters is used currently.
 *
 * @param  desc         pointer to \b struct \b machine_desc
 * @param  tags         pointer to \b struct \b tag
 * @param  cmdline      pointer to the command line
 * @param  mi           pointer to \b struct \b meminfo
 */
static void __init fixup_mxc_board(struct machine_desc *desc, struct tag *tags,
				   char **cmdline, struct meminfo *mi)
{
#ifdef CONFIG_KGDB_8250
	int i;
	for (i = 0;
	     i <
	     (sizeof(serial_platform_data) / sizeof(serial_platform_data[0]));
	     i += 1)
		kgdb8250_add_platform_port(i, &serial_platform_data[i]);
#endif

	mxc_cpu_init();
	
#ifdef CONFIG_MACH_LAB126
	// Tell the system that we only have one bank of contiguious RAM (bank 0)
	// so that we can reserve parts of it to persist across reboots.
	//
	mi->bank[0].start = PLFRM_MEM_BASE;
	mi->bank[0].size = PLFRM_MEM_SIZE;
	mi->nr_banks = 1;
#else
#ifdef CONFIG_DISCONTIGMEM
	do {
		int nid;
		mi->nr_banks = MXC_NUMNODES;
		for (nid = 0; nid < mi->nr_banks; nid++) {
			SET_NODE(mi, nid);
		}
	} while (0);
#endif
#endif
}

#ifdef CONFIG_MACH_LAB126
static void __init ads_mxc_init_irq(void)
{
	// Note: We do this in the IRQ init prior to init'ing the IRQs so that
	// interrupts are still disabled prior to our changing the memory
	// layout.
	//
	reserve_bootmem(BOOT_GLOBALS_BASE, BOOT_GLOBALS_SIZE);
	reserve_bootmem(OOPS_SAVE_BASE, OOPS_SAVE_SIZE);
	
	mxc_init_irq();
}
#endif

#if defined(CONFIG_MXC_PMIC_MC13783) && defined(CONFIG_SND_MXC_PMIC)
extern void gpio_activate_audio_ports(void);

static void __init mxc_init_pmic_audio(void)
{
	struct clk *pll_clk;
	struct clk *ssi_clk;
	struct clk *ckih_clk;
	struct clk *cko_clk;

	/* Enable 26 mhz clock on CKO1 for PMIC audio */
	ckih_clk = clk_get(NULL, "ckih");
	cko_clk = clk_get(NULL, "cko1_clk");
	if (IS_ERR(ckih_clk) || IS_ERR(cko_clk)) {
		printk(KERN_ERR "Unable to set CKO1 output to CKIH\n");
	} else {
		clk_set_parent(cko_clk, ckih_clk);
		clk_set_rate(cko_clk, clk_get_rate(ckih_clk));
		clk_enable(cko_clk);
	}
	clk_put(ckih_clk);
	clk_put(cko_clk);

	/* Assign USBPLL to be used by SSI1/2 */
	pll_clk = clk_get(NULL, "usb_pll");
	ssi_clk = clk_get(NULL, "ssi_clk.0");
	clk_set_parent(ssi_clk, pll_clk);
	clk_enable(ssi_clk);
	clk_put(ssi_clk);

	ssi_clk = clk_get(NULL, "ssi_clk.1");
	clk_set_parent(ssi_clk, pll_clk);
	clk_enable(ssi_clk);
	clk_put(ssi_clk);
	clk_put(pll_clk);

	gpio_activate_audio_ports();
}
#else
static void __inline mxc_init_pmic_audio(void)
{
}
#endif

/*!
 * Board specific initialization.
 */
static void __init mxc_board_init(void)
{
	mxc_cpu_common_init();
	mxc_clocks_init();
	early_console_setup(saved_command_line);
	mxc_init_pmic_audio();
	mxc_gpio_init();
	mx31ads_gpio_init();
	mxc_expio_init();
	mxc_init_keypad();
	mxc_init_extuart();
	mxc_init_nor_mtd();
	mxc_init_nand_mtd();

	spi_register_board_info(mxc_spi_board_info,
				ARRAY_SIZE(mxc_spi_board_info));

	mxc_init_fb();
	mxc_init_ir();
}

#define PLL_PCTL_REG(pd, mfd, mfi, mfn)		\
	((((pd) - 1) << 26) + (((mfd) - 1) << 16) + ((mfi)  << 10) + mfn)

/* For 26MHz input clock */
#define PLL_532MHZ		PLL_PCTL_REG(1, 13, 10, 3)
#define PLL_399MHZ		PLL_PCTL_REG(1, 52, 7, 35)
#define PLL_133MHZ		PLL_PCTL_REG(2, 26, 5, 3)

/* For 27MHz input clock */
#define PLL_532_8MHZ		PLL_PCTL_REG(1, 15, 9, 13)
#define PLL_399_6MHZ		PLL_PCTL_REG(1, 18, 7, 7)
#define PLL_133_2MHZ		PLL_PCTL_REG(3, 5, 7, 2)

#define PDR0_REG(mcu, max, hsp, ipg, nfc)	\
	(MXC_CCM_PDR0_MCU_DIV_##mcu | MXC_CCM_PDR0_MAX_DIV_##max | \
	 MXC_CCM_PDR0_HSP_DIV_##hsp | MXC_CCM_PDR0_IPG_DIV_##ipg | \
	 MXC_CCM_PDR0_NFC_DIV_##nfc)

/* working point(wp): 0 - 133MHz; 1 - 266MHz; 2 - 399MHz; 3 - 532MHz */
/* 26MHz input clock table */
static struct cpu_wp cpu_wp_26[] = {
	{
	 .pll_reg = PLL_532MHZ,
	 .pll_rate = 532000000,
	 .cpu_rate = 133000000,
	 .pdr0_reg = PDR0_REG(4, 4, 4, 2, 6),},
	{
	 .pll_reg = PLL_532MHZ,
	 .pll_rate = 532000000,
	 .cpu_rate = 266000000,
	 .pdr0_reg = PDR0_REG(2, 4, 4, 2, 6),},
	{
	 .pll_reg = PLL_399MHZ,
	 .pll_rate = 399000000,
	 .cpu_rate = 399000000,
	 .pdr0_reg = PDR0_REG(1, 3, 3, 2, 6),},
	{
	 .pll_reg = PLL_532MHZ,
	 .pll_rate = 532000000,
	 .cpu_rate = 532000000,
	 .pdr0_reg = PDR0_REG(1, 4, 4, 2, 6),},
};

/* 27MHz input clock table */
static struct cpu_wp cpu_wp_27[] = {
	{
	 .pll_reg = PLL_532_8MHZ,
	 .pll_rate = 532800000,
	 .cpu_rate = 133200000,
	 .pdr0_reg = PDR0_REG(4, 4, 4, 2, 6),},
	{
	 .pll_reg = PLL_532_8MHZ,
	 .pll_rate = 532800000,
	 .cpu_rate = 266400000,
	 .pdr0_reg = PDR0_REG(2, 4, 4, 2, 6),},
	{
	 .pll_reg = PLL_399_6MHZ,
	 .pll_rate = 399600000,
	 .cpu_rate = 399600000,
	 .pdr0_reg = PDR0_REG(1, 3, 3, 2, 6),},
	{
	 .pll_reg = PLL_532_8MHZ,
	 .pll_rate = 532800000,
	 .cpu_rate = 532800000,
	 .pdr0_reg = PDR0_REG(1, 4, 4, 2, 6),},
};

struct cpu_wp *get_cpu_wp(int *wp)
{
	*wp = 4;
	if ((__raw_readw(PBC_BASE_ADDRESS + PBC_BSTAT) &
	     CKIH_27MHZ_BIT_SET) != 0) {
		return cpu_wp_27;
	} else {
		return cpu_wp_26;
	}
}

/*
 * The following uses standard kernel macros define in arch.h in order to
 * initialize __mach_desc_MX31ADS data structure.
 */
/* *INDENT-OFF* */
MACHINE_START(MX31ADS, "Freescale MX31/MX32 ADS")
	/* Maintainer: Freescale Semiconductor, Inc. */
#ifdef CONFIG_SERIAL_8250_CONSOLE
	.phys_io = CS4_BASE_ADDR,
	.io_pg_offst = ((CS4_BASE_ADDR_VIRT) >> 18) & 0xfffc,
#else
	.phys_io = AIPS1_BASE_ADDR,
	.io_pg_offst = ((AIPS1_BASE_ADDR_VIRT) >> 18) & 0xfffc,
#endif
	.boot_params = PHYS_OFFSET + 0x100,
	.fixup = fixup_mxc_board,
	.map_io = mxc_map_io,
#ifdef CONFIG_MACH_LAB126
	.init_irq = ads_mxc_init_irq,
#else
	.init_irq = mxc_init_irq,
#endif
	.init_machine = mxc_board_init,
	.timer = &mxc_timer,
MACHINE_END

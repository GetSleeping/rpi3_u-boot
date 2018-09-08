/*
 * (C) Copyright 2012 Stephen Warren
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <video.h>
#include <asm/arch/mbox.h>
#include <asm/arch/msg.h>

static int bcm2835_video_probe(struct udevice *dev)
{
	struct video_uc_platdata *plat = dev_get_uclass_platdata(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	int ret;
	int w, h, pitch;
	ulong fb_base = 0, fb_size = 0, fb_start = 0, fb_end = 0;

	debug("bcm2835: Query resolution...\n");
	ret = bcm2835_get_video_size(&w, &h);
	if (ret)
		return -EIO;

	debug("bcm2835: Setting up display for %d x %d\n", w, h);
	ret = bcm2835_set_video_params(&w, &h, 32, BCM2835_MBOX_PIXEL_ORDER_RGB,
				       BCM2835_MBOX_ALPHA_MODE_IGNORED,
				       &fb_base, &fb_size, &pitch);

	debug("bcm2835: Final resolution is %d x %d\n", w, h);

	debug("bcm2835: fb_base is %d.",fb_base);
	debug("bcm2835: fb_size is %d.",fb_size);
	debug("bcm2835: fb_start is %d.",fb_start);
	debug("bcm2835: fb_end is %d.",fb_end);

	/* Enable dcache for the frame buffer */
	fb_start = fb_base & ~(MMU_SECTION_SIZE - 1);
	fb_end = fb_base + fb_size;
	fb_end = ALIGN(fb_end, 1 << MMU_SECTION_SHIFT);
	mmu_set_region_dcache_behaviour(fb_start, fb_end - fb_start,
					DCACHE_WRITEBACK);
	video_set_flush_dcache(dev, false);

	uc_priv->xsize = w;
	uc_priv->ysize = h;
	uc_priv->bpix = VIDEO_BPP32;
	plat->base = fb_base;
	plat->size = fb_size;

	return 0;
}

static const struct udevice_id bcm2835_video_ids[] = {
	{ .compatible = "brcm,bcm2835-hdmi" },
	{ }
};

U_BOOT_DRIVER(bcm2835_video) = {
	.name	= "bcm2835_video",
	.id	= UCLASS_VIDEO,
	.of_match = bcm2835_video_ids,
	.probe	= bcm2835_video_probe,
};

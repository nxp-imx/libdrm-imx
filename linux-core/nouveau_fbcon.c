/*
 * Copyright © 2007 David Airlie
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *     David Airlie
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/slab.h>
#include <linux/sysrq.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>

#include "drmP.h"
#include "drm.h"
#include "drm_crtc.h"
#include "drm_crtc_helper.h"
#include "nouveau_drv.h"
#include "nouveau_drm.h"
#include "nouveau_crtc.h"
#include "nouveau_fb.h"
#include "nouveau_fbcon.h"

extern int nouveau_fbpercrtc;

static int nouveau_fbcon_setcolreg(unsigned regno, unsigned red, unsigned green,
				   unsigned blue, unsigned transp,
				   struct fb_info *info)
{
	struct nouveau_fbcon_par *par = info->par;
	struct drm_device *dev = par->dev;
	struct drm_crtc *crtc;
	int i;

	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		struct nouveau_crtc *nouveau_crtc = to_nouveau_crtc(crtc);
		struct drm_mode_set *modeset = &nouveau_crtc->mode_set;
		struct drm_framebuffer *fb = modeset->fb;

		for (i = 0; i < par->crtc_count; i++)
			if (crtc->base.id == par->crtc_ids[i])
				break;

		if (i == par->crtc_count)
			continue;


		if (regno > 255)
			return 1;

		if (fb->depth == 8) {
			/* TODO */
		}

		if (regno < 16) {
			switch (fb->depth) {
			case 15:
				fb->pseudo_palette[regno] = ((red & 0xf800) >> 1) |
					((green & 0xf800) >>  6) |
					((blue & 0xf800) >> 11);
				break;
			case 16:
				fb->pseudo_palette[regno] = (red & 0xf800) |
					((green & 0xfc00) >>  5) |
					((blue  & 0xf800) >> 11);
				break;
			case 24:
			case 32:
				fb->pseudo_palette[regno] = ((red & 0xff00) << 8) |
					(green & 0xff00) |
					((blue  & 0xff00) >> 8);
				break;
			}
		}
	}
	return 0;
}

static int nouveau_fbcon_check_var(struct fb_var_screeninfo *var,
				   struct fb_info *info)
{
	struct nouveau_fbcon_par *par = info->par;
	struct nouveau_framebuffer *nouveau_fb = par->nouveau_fb;
	struct drm_framebuffer *fb = &nouveau_fb->base;
	int depth;

	if (var->pixclock == -1 || !var->pixclock)
		return -EINVAL;

	/* Need to resize the fb object !!! */
	if (var->xres > fb->width || var->yres > fb->height) {
		DRM_ERROR("Requested width/height is greater than current fb object %dx%d > %dx%d\n",var->xres,var->yres,fb->width,fb->height);
		DRM_ERROR("Need resizing code.\n");
		return -EINVAL;
	}

	switch (var->bits_per_pixel) {
	case 16:
		depth = (var->green.length == 6) ? 16 : 15;
		break;
	case 32:
		depth = (var->transp.length > 0) ? 32 : 24;
		break;
	default:
		depth = var->bits_per_pixel;
		break;
	}

	switch (depth) {
	case 8:
		var->red.offset = 0;
		var->green.offset = 0;
		var->blue.offset = 0;
		var->red.length = 8;
		var->green.length = 8;
		var->blue.length = 8;
		var->transp.length = 0;
		var->transp.offset = 0;
		break;
	case 15:
		var->red.offset = 10;
		var->green.offset = 5;
		var->blue.offset = 0;
		var->red.length = 5;
		var->green.length = 5;
		var->blue.length = 5;
		var->transp.length = 1;
		var->transp.offset = 15;
		break;
	case 16:
		var->red.offset = 11;
		var->green.offset = 5;
		var->blue.offset = 0;
		var->red.length = 5;
		var->green.length = 6;
		var->blue.length = 5;
		var->transp.length = 0;
		var->transp.offset = 0;
		break;
	case 24:
		var->red.offset = 16;
		var->green.offset = 8;
		var->blue.offset = 0;
		var->red.length = 8;
		var->green.length = 8;
		var->blue.length = 8;
		var->transp.length = 0;
		var->transp.offset = 0;
		break;
	case 32:
		var->red.offset = 16;
		var->green.offset = 8;
		var->blue.offset = 0;
		var->red.length = 8;
		var->green.length = 8;
		var->blue.length = 8;
		var->transp.length = 8;
		var->transp.offset = 24;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

/* this will let fbcon do the mode init */
/* FIXME: take mode config lock? */
static int nouveau_fbcon_set_par(struct fb_info *info)
{
	struct nouveau_fbcon_par *par = info->par;
	struct drm_device *dev = par->dev;
	struct fb_var_screeninfo *var = &info->var;
	int i;

	DRM_DEBUG("%d %d\n", var->xres, var->pixclock);

	if (var->pixclock != -1) {

		DRM_ERROR("PIXEL CLCOK SET\n");
		return -EINVAL;
	} else {
		struct drm_crtc *crtc;
		int ret;

		list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
			struct nouveau_crtc *nouveau_crtc = to_nouveau_crtc(crtc);

			for (i = 0; i < par->crtc_count; i++)
				if (crtc->base.id == par->crtc_ids[i])
					break;

			if (i == par->crtc_count)
				continue;

			if (crtc->fb == nouveau_crtc->mode_set.fb) {
				mutex_lock(&dev->mode_config.mutex);
				ret = crtc->funcs->set_config(&nouveau_crtc->mode_set);
				mutex_unlock(&dev->mode_config.mutex);
				if (ret)
					return ret;
			}
		}
		return 0;
	}
}

static int nouveau_fbcon_pan_display(struct fb_var_screeninfo *var,
				     struct fb_info *info)
{
	struct nouveau_fbcon_par *par = info->par;
	struct drm_device *dev = par->dev;
	struct drm_mode_set *modeset;
	struct drm_crtc *crtc;
	struct nouveau_crtc *nouveau_crtc;
	int ret = 0;
	int i;

	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		for (i = 0; i < par->crtc_count; i++)
			if (crtc->base.id == par->crtc_ids[i])
				break;

		if (i == par->crtc_count)
			continue;

		nouveau_crtc = to_nouveau_crtc(crtc);
		modeset = &nouveau_crtc->mode_set;

		modeset->x = var->xoffset;
		modeset->y = var->yoffset;

		if (modeset->num_connectors) {
			mutex_lock(&dev->mode_config.mutex);
			ret = crtc->funcs->set_config(modeset);
			mutex_unlock(&dev->mode_config.mutex);
			if (!ret) {
				info->var.xoffset = var->xoffset;
				info->var.yoffset = var->yoffset;
			}
		}
	}

	return ret;
}

static void nouveau_fbcon_on(struct fb_info *info)
{
	struct nouveau_fbcon_par *par = info->par;
	struct drm_device *dev = par->dev;
	struct drm_crtc *crtc;
	struct drm_encoder *encoder;
	int i;

	/*
	 * For each CRTC in this fb, find all associated encoders
	 * and turn them off, then turn off the CRTC.
	 */
	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		struct drm_crtc_helper_funcs *crtc_funcs = crtc->helper_private;

		for (i = 0; i < par->crtc_count; i++)
			if (crtc->base.id == par->crtc_ids[i])
				break;

		crtc_funcs->dpms(crtc, DRM_MODE_DPMS_ON);

		/* Found a CRTC on this fb, now find encoders */
		list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {
			if (encoder->crtc == crtc) {
				struct drm_encoder_helper_funcs *encoder_funcs;
				encoder_funcs = encoder->helper_private;
				encoder_funcs->dpms(encoder, DRM_MODE_DPMS_ON);
			}
		}
	}
}

static void nouveau_fbcon_off(struct fb_info *info, int dpms_mode)
{
	struct nouveau_fbcon_par *par = info->par;
	struct drm_device *dev = par->dev;
	struct drm_crtc *crtc;
	struct drm_encoder *encoder;
	int i;

	/*
	 * For each CRTC in this fb, find all associated encoders
	 * and turn them off, then turn off the CRTC.
	 */
	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		struct drm_crtc_helper_funcs *crtc_funcs = crtc->helper_private;

		for (i = 0; i < par->crtc_count; i++)
			if (crtc->base.id == par->crtc_ids[i])
				break;

		/* Found a CRTC on this fb, now find encoders */
		list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {
			if (encoder->crtc == crtc) {
				struct drm_encoder_helper_funcs *encoder_funcs;
				encoder_funcs = encoder->helper_private;
				encoder_funcs->dpms(encoder, dpms_mode);
			}
		}
		if (dpms_mode == DRM_MODE_DPMS_OFF)
			crtc_funcs->dpms(crtc, dpms_mode);
	}
}

static int nouveau_fbcon_blank(int blank, struct fb_info *info)
{
	switch (blank) {
	case FB_BLANK_UNBLANK:
		nouveau_fbcon_on(info);
		break;
	case FB_BLANK_NORMAL:
		nouveau_fbcon_off(info, DRM_MODE_DPMS_STANDBY);
		break;
	case FB_BLANK_HSYNC_SUSPEND:
		nouveau_fbcon_off(info, DRM_MODE_DPMS_STANDBY);
		break;
	case FB_BLANK_VSYNC_SUSPEND:
		nouveau_fbcon_off(info, DRM_MODE_DPMS_SUSPEND);
		break;
	case FB_BLANK_POWERDOWN:
		nouveau_fbcon_off(info, DRM_MODE_DPMS_OFF);
		break;
	}
	return 0;
}

static struct fb_ops nouveau_fbcon_ops = {
	.owner = THIS_MODULE,
	.fb_check_var = nouveau_fbcon_check_var,
	.fb_set_par = nouveau_fbcon_set_par,
	.fb_setcolreg = nouveau_fbcon_setcolreg,
	.fb_fillrect = cfb_fillrect,
	.fb_copyarea = cfb_copyarea,
	.fb_imageblit = cfb_imageblit,
	.fb_pan_display = nouveau_fbcon_pan_display,
	.fb_blank = nouveau_fbcon_blank,
};

/**
 * Curretly it is assumed that the old framebuffer is reused.
 *
 * LOCKING
 * caller should hold the mode config lock.
 *
 */
int nouveau_fbcon_resize(struct drm_device *dev, struct drm_crtc *crtc)
{
	struct fb_info *info;
	struct drm_framebuffer *fb;
	struct drm_display_mode *mode = crtc->desired_mode;

	fb = crtc->fb;
	if (!fb)
		return 1;

	info = fb->fbdev;
	if (!info)
		return 1;

	if (!mode)
		return 1;

	info->var.xres = mode->hdisplay;
	info->var.right_margin = mode->hsync_start - mode->hdisplay;
	info->var.hsync_len = mode->hsync_end - mode->hsync_start;
	info->var.left_margin = mode->htotal - mode->hsync_end;
	info->var.yres = mode->vdisplay;
	info->var.lower_margin = mode->vsync_start - mode->vdisplay;
	info->var.vsync_len = mode->vsync_end - mode->vsync_start;
	info->var.upper_margin = mode->vtotal - mode->vsync_end;
	info->var.pixclock = 10000000 / mode->htotal * 1000 / mode->vtotal * 100;
	/* avoid overflow */
	info->var.pixclock = info->var.pixclock * 1000 / mode->vrefresh;

	return 0;
}
EXPORT_SYMBOL(nouveau_fbcon_resize);

static struct drm_mode_set kernelfb_mode;

static int nouveau_fbcon_panic(struct notifier_block *n, unsigned long ununsed,
			       void *panic_str)
{
	DRM_ERROR("panic occurred, switching back to text console\n");

	nouveau_fbcon_restore();
	return 0;
}

static struct notifier_block paniced = {
	.notifier_call = nouveau_fbcon_panic,
};

static int nouveau_fbcon_create(struct drm_device *dev, uint32_t fb_width,
				uint32_t fb_height, uint32_t surface_width,
				uint32_t surface_height,
				struct nouveau_framebuffer **nouveau_fb_p)
{
	struct drm_nouveau_private *dev_priv = dev->dev_private;
	struct fb_info *info;
	struct nouveau_fbcon_par *par;
	struct drm_framebuffer *fb;
	struct nouveau_framebuffer *nouveau_fb;
	struct drm_mode_fb_cmd mode_cmd;
	struct drm_gem_object *gem = NULL;
	struct nouveau_gem_object *ngem;
	struct device *device = &dev->pdev->dev;
	struct fb_fillrect rect;
	int size, ret;

	mode_cmd.width = surface_width;
	mode_cmd.height = surface_height;

	mode_cmd.bpp = 32;
	mode_cmd.pitch = ALIGN(mode_cmd.width, 64) * ((mode_cmd.bpp + 1) / 8);
	mode_cmd.depth = 24;

	size = mode_cmd.pitch * mode_cmd.height;
	size = ALIGN(size, PAGE_SIZE);

	ret = nouveau_gem_new(dev, NULL, size, 0, NOUVEAU_GEM_DOMAIN_VRAM, &gem);
	if (ret) {
		printk(KERN_ERR "failed to allocate framebuffer\n");
		ret = -ENOMEM;
		goto out;
	}
	ngem = gem->driver_private;

	ret = nouveau_gem_pin(gem, NOUVEAU_GEM_DOMAIN_VRAM);
	mutex_lock(&dev->struct_mutex);
	if (ret) {
		DRM_ERROR("failed to pin fb: %d\n", ret);
		goto out_unref;
	}

	fb = nouveau_framebuffer_create(dev, gem, &mode_cmd);
	if (!fb) {
		ret = -ENOMEM;
		DRM_ERROR("failed to allocate fb.\n");
		goto out_unref;
	}

	list_add(&fb->filp_head, &dev->mode_config.fb_kernel_list);

	nouveau_fb = to_nouveau_framebuffer(fb);
	*nouveau_fb_p = nouveau_fb;

	ret = drm_bo_kmap(ngem->bo, 0, ngem->bo->mem.num_pages,
			  &nouveau_fb->kmap);
	if (ret) {
		DRM_ERROR("failed to kmap fb: %d\n", ret);
		goto out_unref;
	}

	info = framebuffer_alloc(sizeof(struct nouveau_fbcon_par), device);
	if (!info) {
		ret = -ENOMEM;
		goto out_unref;
	}

	par = info->par;

	strcpy(info->fix.id, "nouveaudrmfb");
	info->fix.type = FB_TYPE_PACKED_PIXELS;
	info->fix.visual = FB_VISUAL_TRUECOLOR;
	info->fix.type_aux = 0;
	info->fix.xpanstep = 1; /* doing it in hw */
	info->fix.ypanstep = 1; /* doing it in hw */
	info->fix.ywrapstep = 0;
	info->fix.accel = FB_ACCEL_I830;
	info->fix.type_aux = 0;

	info->flags = FBINFO_DEFAULT;

	info->fbops = &nouveau_fbcon_ops;

	info->fix.line_length = fb->pitch;
	info->fix.smem_start = dev->mode_config.fb_base + ngem->bo->offset -
			       dev_priv->vm_vram_base;
	info->fix.smem_len = size;

	info->flags = FBINFO_DEFAULT;

	info->screen_base = nouveau_fb->kmap.virtual;
	info->screen_size = size;

//	memset(info->screen_base, 0, size);

	info->pseudo_palette = fb->pseudo_palette;
	info->var.xres_virtual = fb->width;
	info->var.yres_virtual = fb->height;
	info->var.bits_per_pixel = fb->bits_per_pixel;
	info->var.xoffset = 0;
	info->var.yoffset = 0;
	info->var.activate = FB_ACTIVATE_NOW;
	info->var.height = -1;
	info->var.width = -1;

	info->var.xres = fb_width;
	info->var.yres = fb_height;

	/* FIXME: we really shouldn't expose mmio space at all */
	info->fix.mmio_start = pci_resource_start(dev->pdev, 1);
	info->fix.mmio_len = pci_resource_len(dev->pdev, 1);

	info->pixmap.size = 64*1024;
	info->pixmap.buf_align = 8;
	info->pixmap.access_align = 32;
	info->pixmap.flags = FB_PIXMAP_SYSTEM;
	info->pixmap.scan_align = 1;

	switch(fb->depth) {
	case 8:
		info->var.red.offset = 0;
		info->var.green.offset = 0;
		info->var.blue.offset = 0;
		info->var.red.length = 8; /* 8bit DAC */
		info->var.green.length = 8;
		info->var.blue.length = 8;
		info->var.transp.offset = 0;
		info->var.transp.length = 0;
		break;
	case 15:
		info->var.red.offset = 10;
		info->var.green.offset = 5;
		info->var.blue.offset = 0;
		info->var.red.length = 5;
		info->var.green.length = 5;
		info->var.blue.length = 5;
		info->var.transp.offset = 15;
		info->var.transp.length = 1;
		break;
	case 16:
		info->var.red.offset = 11;
		info->var.green.offset = 5;
		info->var.blue.offset = 0;
		info->var.red.length = 5;
		info->var.green.length = 6;
		info->var.blue.length = 5;
		info->var.transp.offset = 0;
		break;
	case 24:
		info->var.red.offset = 16;
		info->var.green.offset = 8;
		info->var.blue.offset = 0;
		info->var.red.length = 8;
		info->var.green.length = 8;
		info->var.blue.length = 8;
		info->var.transp.offset = 0;
		info->var.transp.length = 0;
		break;
	case 32:
		info->var.red.offset = 16;
		info->var.green.offset = 8;
		info->var.blue.offset = 0;
		info->var.red.length = 8;
		info->var.green.length = 8;
		info->var.blue.length = 8;
		info->var.transp.offset = 24;
		info->var.transp.length = 8;
		break;
	default:
		break;
	}

	fb->fbdev = info;

	par->nouveau_fb = nouveau_fb;
	par->dev = dev;

	switch (dev_priv->card_type) {
	case NV_50:
		nv50_fbcon_accel_init(info);
		break;
	default:
		break;
	};

	/* Clear the entire fbcon.  The drm will program every connector
	 * with it's preferred mode.  If the sizes differ, one display will
	 * quite likely have garbage around the console.
	 */
	rect.dx = rect.dy = 0;
	rect.width = surface_width;
	rect.height = surface_height;
	rect.color = 0;
	rect.rop = ROP_COPY;
	info->fbops->fb_fillrect(info, &rect);

	/* To allow resizeing without swapping buffers */
	printk("allocated %dx%d fb: 0x%lx, bo %p\n", nouveau_fb->base.width,
	       nouveau_fb->base.height, ngem->bo->offset, gem);

	mutex_unlock(&dev->struct_mutex);
	return 0;

out_unref:
	drm_gem_object_unreference(gem);
	mutex_unlock(&dev->struct_mutex);
out:
	return ret;
}

static int nouveau_fbcon_multi_fb_probe_crtc(struct drm_device *dev,
					     struct drm_crtc *crtc)
{
	struct nouveau_crtc *nouveau_crtc = to_nouveau_crtc(crtc);
	struct nouveau_framebuffer *nouveau_fb;
	struct drm_framebuffer *fb;
	struct drm_connector *connector;
	struct fb_info *info;
	struct nouveau_fbcon_par *par;
	struct drm_mode_set *modeset;
	unsigned int width, height;
	int new_fb = 0;
	int ret, i, conn_count;

	if (!drm_helper_crtc_in_use(crtc))
		return 0;

	if (!crtc->desired_mode)
		return 0;

	width = crtc->desired_mode->hdisplay;
	height = crtc->desired_mode->vdisplay;

	/* is there an fb bound to this crtc already */
	if (!nouveau_crtc->mode_set.fb) {
		ret = nouveau_fbcon_create(dev, width, height, width, height, &nouveau_fb);
		if (ret)
			return -EINVAL;
		new_fb = 1;
	} else {
		fb = nouveau_crtc->mode_set.fb;
		nouveau_fb = to_nouveau_framebuffer(fb);
		if ((nouveau_fb->base.width < width) || (nouveau_fb->base.height < height))
			return -EINVAL;
	}

	info = nouveau_fb->base.fbdev;
	par = info->par;

	modeset = &nouveau_crtc->mode_set;
	modeset->fb = &nouveau_fb->base;
	conn_count = 0;
	list_for_each_entry(connector, &dev->mode_config.connector_list, head) {
		if (connector->encoder) {
			if (connector->encoder->crtc == modeset->crtc) {
				modeset->connectors[conn_count] = connector;
				conn_count++;
				if (conn_count > NOUVEAUFB_CONN_LIMIT)
					BUG();
			}
		}
	}

	for (i = conn_count; i < NOUVEAUFB_CONN_LIMIT; i++)
		modeset->connectors[i] = NULL;

	par->crtc_ids[0] = crtc->base.id;

	modeset->num_connectors = conn_count;
	if (modeset->mode != modeset->crtc->desired_mode)
		modeset->mode = modeset->crtc->desired_mode;

	par->crtc_count = 1;

	if (new_fb) {
		info->var.pixclock = -1;
		if (register_framebuffer(info) < 0)
			return -EINVAL;
	} else
		nouveau_fbcon_set_par(info);

	printk(KERN_INFO "fb%d: %s frame buffer device\n", info->node,
	       info->fix.id);

	/* Switch back to kernel console on panic */
	kernelfb_mode = *modeset;
	atomic_notifier_chain_register(&panic_notifier_list, &paniced);
	printk(KERN_INFO "registered panic notifier\n");

	return 0;
}

static int nouveau_fbcon_multi_fb_probe(struct drm_device *dev)
{

	struct drm_crtc *crtc;
	int ret = 0;

	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		ret = nouveau_fbcon_multi_fb_probe_crtc(dev, crtc);
		if (ret)
			return ret;
	}
	return ret;
}

static int nouveau_fbcon_single_fb_probe(struct drm_device *dev)
{
	struct drm_crtc *crtc;
	struct drm_connector *connector;
	unsigned int fb_width = (unsigned)-1, fb_height = (unsigned)-1;
	unsigned int surface_width = 0, surface_height = 0;
	int new_fb = 0;
	int crtc_count = 0;
	int ret, i, conn_count = 0;
	struct nouveau_framebuffer *nouveau_fb;
	struct fb_info *info;
	struct nouveau_fbcon_par *par;
	struct drm_mode_set *modeset = NULL;

	DRM_DEBUG("\n");

	/* Get a count of crtcs now in use and new min/maxes width/heights */
	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		if (!drm_helper_crtc_in_use(crtc))
			continue;

		crtc_count++;
		if (!crtc->desired_mode)
			continue;

		/* Smallest mode determines console size... */
		if (crtc->desired_mode->hdisplay < fb_width)
			fb_width = crtc->desired_mode->hdisplay;

		if (crtc->desired_mode->vdisplay < fb_height)
			fb_height = crtc->desired_mode->vdisplay;

		/* ... but largest for memory allocation dimensions */
		if (crtc->desired_mode->hdisplay > surface_width)
			surface_width = crtc->desired_mode->hdisplay;

		if (crtc->desired_mode->vdisplay > surface_height)
			surface_height = crtc->desired_mode->vdisplay;
	}

	if (crtc_count == 0 || fb_width == -1 || fb_height == -1) {
		/* hmm everyone went away - assume VGA cable just fell out
		   and will come back later. */
		DRM_DEBUG("no CRTCs available?\n");
		return 0;
	}

//fail
	/* Find the fb for our new config */
	if (list_empty(&dev->mode_config.fb_kernel_list)) {
		DRM_DEBUG("creating new fb (console size %dx%d, "
			  "buffer size %dx%d)\n", fb_width, fb_height,
			  surface_width, surface_height);
		ret = nouveau_fbcon_create(dev, fb_width, fb_height, surface_width,
				     surface_height, &nouveau_fb);
		if (ret)
			return -EINVAL;
		new_fb = 1;
	} else {
		struct drm_framebuffer *fb;

		fb = list_first_entry(&dev->mode_config.fb_kernel_list,
				      struct drm_framebuffer, filp_head);
		nouveau_fb = to_nouveau_framebuffer(fb);

		/* if someone hotplugs something bigger than we have already
		 * allocated, we are pwned.  As really we can't resize an
		 * fbdev that is in the wild currently due to fbdev not really
		 * being designed for the lower layers moving stuff around
		 * under it.
		 * - so in the grand style of things - punt.
		 */
		if ((fb->width < surface_width) ||
		    (fb->height < surface_height)) {
			DRM_ERROR("fb not large enough for console\n");
			return -EINVAL;
		}
	}
// fail

	info = nouveau_fb->base.fbdev;
	par = info->par;

	crtc_count = 0;
	/*
	 * For each CRTC, set up the connector list for the CRTC's mode
	 * set configuration.
	 */
	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		struct nouveau_crtc *nouveau_crtc = to_nouveau_crtc(crtc);

		modeset = &nouveau_crtc->mode_set;
		modeset->fb = &nouveau_fb->base;
		conn_count = 0;
		list_for_each_entry(connector, &dev->mode_config.connector_list,
				    head) {
			if (!connector->encoder)
				continue;

			if(connector->encoder->crtc == modeset->crtc) {
				modeset->connectors[conn_count++] = connector;
				if (conn_count > NOUVEAUFB_CONN_LIMIT)
					BUG();
			}
		}

		/* Zero out remaining connector pointers */
		for (i = conn_count; i < NOUVEAUFB_CONN_LIMIT; i++)
			modeset->connectors[i] = NULL;

		par->crtc_ids[crtc_count++] = crtc->base.id;

		modeset->num_connectors = conn_count;
		if (modeset->mode != modeset->crtc->desired_mode)
			modeset->mode = modeset->crtc->desired_mode;
	}
	par->crtc_count = crtc_count;

	if (new_fb) {
		info->var.pixclock = -1;
		if (register_framebuffer(info) < 0)
			return -EINVAL;
	} else
		nouveau_fbcon_set_par(info);

	printk(KERN_INFO "fb%d: %s frame buffer device\n", info->node,
	       info->fix.id);

	/* Switch back to kernel console on panic */
	kernelfb_mode = *modeset;
	atomic_notifier_chain_register(&panic_notifier_list, &paniced);
	printk(KERN_INFO "registered panic notifier\n");

	return 0;
}

/**
 * nouveau_fbcon_restore - restore the framebuffer console (kernel) config
 *
 * Restore's the kernel's fbcon mode, used for lastclose & panic paths.
 */
void nouveau_fbcon_restore(void)
{
	drm_crtc_helper_set_config(&kernelfb_mode);
}

static void nouveau_fbcon_sysrq(int dummy1, struct tty_struct *dummy3)
{
        nouveau_fbcon_restore();
}

static struct sysrq_key_op sysrq_nouveau_fbcon_restore_op = {
        .handler = nouveau_fbcon_sysrq,
        .help_msg = "force fb",
        .action_msg = "force restore of fb console",
};

int nouveau_fbcon_probe(struct drm_device *dev)
{
	int ret;

	DRM_DEBUG("\n");

	/* something has changed in the lower levels of hell - deal with it
	   here */

	/* two modes : a) 1 fb to rule all crtcs.
	               b) one fb per crtc.
	   two actions 1) new connected device
	               2) device removed.
	   case a/1 : if the fb surface isn't big enough - resize the surface fb.
	              if the fb size isn't big enough - resize fb into surface.
		      if everything big enough configure the new crtc/etc.
	   case a/2 : undo the configuration
	              possibly resize down the fb to fit the new configuration.
           case b/1 : see if it is on a new crtc - setup a new fb and add it.
	   case b/2 : teardown the new fb.
	*/

	/* mode a first */
	/* search for an fb */
	if (nouveau_fbpercrtc) {
		ret = nouveau_fbcon_multi_fb_probe(dev);
	} else {
		ret = nouveau_fbcon_single_fb_probe(dev);
	}

	register_sysrq_key('g', &sysrq_nouveau_fbcon_restore_op);

	return ret;
}
EXPORT_SYMBOL(nouveau_fbcon_probe);

int nouveau_fbcon_remove(struct drm_device *dev, struct drm_framebuffer *fb)
{
	struct nouveau_framebuffer *nouveau_fb = to_nouveau_framebuffer(fb);
	struct fb_info *info;

	if (!fb)
		return -EINVAL;

	info = fb->fbdev;

	if (info) {
		unregister_framebuffer(info);
		drm_bo_kunmap(&nouveau_fb->kmap);
		framebuffer_release(info);
	}

	atomic_notifier_chain_unregister(&panic_notifier_list, &paniced);
	memset(&kernelfb_mode, 0, sizeof(struct drm_mode_set));
	return 0;
}
EXPORT_SYMBOL(nouveau_fbcon_remove);
MODULE_LICENSE("GPL and additional rights");
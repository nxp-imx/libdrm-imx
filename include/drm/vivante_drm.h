/*
 * Copyright © 2017 VIVANTE Corporation
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __VIVNATE_DRM_H__
#define __VIVNATE_DRM_H__

#include <drm.h>

#if defined(__cplusplus)
extern "C" {
#endif

struct drm_viv_gem_create {
    __u64 size;         /* in */
    __u32 flags;        /* in */
    __u32 handle;       /* out */
};

struct drm_viv_gem_lock {
    __u32 handle;
    __u32 cacheable;
    __u32 gpu_va;
    __u64 cpu_va;
};

struct drm_viv_gem_unlock {
    __u32 handle;
};

struct drm_viv_gem_cache {
    __u32 handle;
    __u32 op;
    __u64 logical;
    __u64 bytes;
};

struct drm_viv_gem_getinfo {
    __u32 handle;
    __u32 param;
    __u64 value;
};

#define DRM_VIV_GEM_CREATE          0x00
#define DRM_VIV_GEM_LOCK            0x01
#define DRM_VIV_GEM_UNLOCK          0x02
#define DRM_VIV_GEM_CACHE           0x03
#define DRM_VIV_GEM_GETINFO         0x04
#define DRM_VIV_NUM_IOCTLS          0x05

#define DRM_IOCTL_VIV_GEM_CREATE    DRM_IOWR(DRM_COMMAND_BASE + DRM_VIV_GEM_CREATE,     struct drm_viv_gem_create)
#define DRM_IOCTL_VIV_GEM_LOCK      DRM_IOWR(DRM_COMMAND_BASE + DRM_VIV_GEM_LOCK,       struct drm_viv_gem_lock)
#define DRM_IOCTL_VIV_GEM_UNLOCK    DRM_IOWR(DRM_COMMAND_BASE + DRM_VIV_GEM_UNLOCK,     struct drm_viv_gem_unlock)
#define DRM_IOCTL_VIV_GEM_CACHE     DRM_IOWR(DRM_COMMAND_BASE + DRM_VIV_GEM_CACHE,      struct drm_viv_gem_cache)
#define DRM_IOCTL_VIV_GEM_GETINFO   DRM_IOWR(DRM_COMMAND_BASE + DRM_VIV_GEM_GETINFO,    struct drm_viv_gem_getinfo)

#if defined(__cplusplus)
}
#endif

#endif /* __VIVNATE_DRM_H__ */

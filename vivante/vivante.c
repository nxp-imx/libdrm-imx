/*
 * Copyright © 2012, 2013 Thierry Reding
 * Copyright © 2013 Erik Faye-Lund
 * Copyright © 2014 NVIDIA Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>

#include <xf86drm.h>

#include <vivante_drm.h>

#include "vivante.h"

int drm_vivante_bo_create(int fd, uint32_t flags, uint32_t size, uint32_t *handle)
{
    struct drm_viv_gem_create args;
    int err = 0;

    if (size == 0 || !handle)
        return -EINVAL;

    memset(&args, 0, sizeof(args));
    args.flags = flags;
    args.size = size;

    if (drmIoctl(fd, DRM_IOCTL_VIV_GEM_CREATE, &args))
        return -errno;

    *handle = args.handle;

    return 0;
}

int drm_vivante_bo_destroy(int fd, uint32_t handle)
{
    struct drm_gem_close args;

    if (!handle)
        return -EINVAL;

    args.handle = handle;
    if (drmIoctl(fd, DRM_IOCTL_GEM_CLOSE, &args))
        return -errno;

    return 0;
}

void* drm_vivante_bo_lock(int fd, uint32_t handle, uint32_t *gpu_va)
{
    struct drm_viv_gem_lock args;

    if (!handle)
        return NULL;

    args.handle = handle;
    args.cacheable = 0;
    if (drmIoctl(fd, DRM_IOCTL_VIV_GEM_LOCK, &args))
        return NULL;

    if (gpu_va)
    {
        *gpu_va = args.gpu_va;
    }

    return (void*)(intptr_t)args.cpu_va;
}

int drm_vivante_bo_unlock(int fd, uint32_t handle)
{
    struct drm_viv_gem_unlock args;

    if (!handle)
        return -EINVAL;

    args.handle = handle;
    if (drmIoctl(fd, DRM_IOCTL_VIV_GEM_UNLOCK, &args))
        return -errno;

    return 0;
}

int drm_vivante_bo_cache(int fd, uint32_t handle, uint32_t op, void* logical, uint32_t bytes)
{
    struct drm_viv_gem_cache args;

    if (!handle)
        return -EINVAL;

    args.handle = handle;
    args.op = handle;
    args.logical = (uintptr_t)logical;
    args.bytes = bytes;
    if (drmIoctl(fd, DRM_IOCTL_VIV_GEM_CACHE, &args))
        return -errno;

    return 0;
}

int drm_vivante_bo_getinfo(int fd, uint32_t handle, uint32_t param, uint64_t *value)
{
    struct drm_viv_gem_getinfo args;

    if (!handle)
        return -EINVAL;

    args.handle = handle;
    args.param  = param;
    if (drmIoctl(fd, DRM_IOCTL_VIV_GEM_GETINFO, &args))
        return -errno;

    *value = args.value;

    return 0;
}




/*
 * Copyright © 2017 VIVANTE Corporation
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
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __DRM_VIVANTE_H__
#define __DRM_VIVANTE_H__ 1

#include <stdint.h>
#include <stdlib.h>

enum VIV_CACHE_OP {
    VIV_CACHE_CLEAN      = 0x01,
    VIV_CACHE_INVALIDATE = 0x02,
    VIV_CACHE_FLUSH      = VIV_CACHE_CLEAN  | VIV_CACHE_INVALIDATE,
    VIV_CACHE_BARRIER    = 0x04
};

enum VIV_GEM_PARAM {
    VIV_GEM_PARAM_NODE = 0,
    VIV_GEM_PARAM_POOL,
    VIV_GEM_PARAM_SIZE,
};

int drm_vivante_bo_create(int fd, uint32_t flags, uint32_t size, uint32_t *handle);
int drm_vivante_bo_destroy(int fd, uint32_t handle);
void* drm_vivante_bo_lock(int fd, uint32_t handle, uint32_t *gpu_va);
int drm_vivante_bo_unlock(int fd, uint32_t handle);
int drm_vivante_bo_cache(int fd, uint32_t handle, uint32_t op, void* logical, uint32_t bytes);
int drm_vivante_bo_getinfo(int fd, uint32_t handle, uint32_t param, uint64_t *value);

#endif /* __DRM_VIVANTE_H__ */

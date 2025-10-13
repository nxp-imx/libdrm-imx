/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef _UAPI_IMX_DRM_H_
#define _UAPI_IMX_DRM_H_
#include "drm.h"
#ifdef __cplusplus
extern "C" {
#endif
struct drm_imx_dpu_frame_info {
  __u32 width;
  __u32 height;
  __u32 x_offset;
  __u32 y_offset;
  __u32 stride;
  __u32 format;
  __u64 modifier;
  __u64 baddr;
  __u64 uv_addr;
};
struct drm_imx_dpu_frame_plane_info {
  int src_plane_fd[3];
  __u32 src_plane_offset[3];
  int dst_plane_fd[3];
  __u32 dst_plane_offset[3];
};
#define DRM_IMX_DPU_SET_CMDLIST 0x00
#define DRM_IMX_DPU_WAIT 0x01
#define DRM_IMX_DPU_GET_PARAM 0x02
#define DRM_IMX_DPU_SYNC_DMABUF 0x03
#define DRM_IOCTL_IMX_DPU_SET_CMDLIST DRM_IOWR(DRM_COMMAND_BASE + DRM_IMX_DPU_SET_CMDLIST, struct drm_imx_dpu_set_cmdlist)
#define DRM_IOCTL_IMX_DPU_WAIT DRM_IOWR(DRM_COMMAND_BASE + DRM_IMX_DPU_WAIT, struct drm_imx_dpu_wait)
#define DRM_IOCTL_IMX_DPU_GET_PARAM DRM_IOWR(DRM_COMMAND_BASE + DRM_IMX_DPU_GET_PARAM, enum drm_imx_dpu_param)
#define DRM_IOCTL_IMX_DPU_SYNC_DMABUF DRM_IOW(DRM_COMMAND_BASE + DRM_IMX_DPU_SYNC_DMABUF, struct drm_imx_dpu_sync_dmabuf)
struct drm_imx_dpu_set_cmdlist {
  __u64 cmd;
  __u32 cmd_nr;
  __u64 user_data;
};
struct drm_imx_dpu_wait {
  __u64 user_data;
};
enum drm_imx_dpu_sync_direction {
  IMX_DPU_SYNC_TO_CPU = 0,
  IMX_DPU_SYNC_TO_DEVICE = 1,
  IMX_DPU_SYNC_TO_BOTH = 2,
};
struct drm_imx_dpu_sync_dmabuf {
  __u32 dmabuf_fd;
  __u32 direction;
};
enum drm_imx_dpu_param {
  DRM_IMX_MAX_DPUS,
  DRM_IMX_GET_FENCE,
};
#ifdef __cplusplus
}
#endif
#endif

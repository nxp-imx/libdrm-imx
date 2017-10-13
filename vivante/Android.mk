LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libdrm_vivante

LOCAL_SHARED_LIBRARIES := libdrm

LOCAL_SRC_FILES := vivante.c

include $(LIBDRM_COMMON_MK)
include $(BUILD_SHARED_LIBRARY)

include ${SDK_DIR}/product/configs/${MFRS_CFG}

include ${SDK_DIR}/build/script/env.mk

include ${SDK_DIR}/build/script/kern_lib.mk

MT_STATIC_LIB_CFLAGS = -ffunction-sections -fdata-sections
MT_SHARED_LIB_CFLAGS = -fPIC
MT_SHARED_LIB_LDFLAGS = -shared
MT_APP_CFLAGS = -ffunction-sections -fdata-sections
MT_APP_LDFLAGS = -Wl,--gc-sections

CFG_MT_CFLAGS =
CFG_MT_KMOD_CFLAGS =

ifeq ($(CONFIG_MT_OPTIMIZE_OS),y)
CFG_MT_CFLAGS += -Os
else ifeq ($(CONFIG_MT_OPTIMIZE_O2),y)
CFG_MT_CFLAGS += -O2
else
CFG_MT_CFLAGS += -O0
endif
#don't use -ffast-math, is not safe, it will not check nan

ifeq ($(CONFIG_MT_DEBUG),y)
CFG_MT_CFLAGS += -g
endif

ifeq ($(CONFIG_MT_INSTRUMENT_FUNCTIONS),y)
CFG_MT_CFLAGS += -finstrument-functions
endif

CFG_MT_CFLAGS += -fno-strict-aliasing

CFG_MT_YES_WARN_OPTION = -Wall -Wformat=2 -Wstrict-aliasing=2 -Wstrict-prototypes -Wshadow -Wwrite-strings -Wfloat-equal -Wundef -Wunreachable-code -Wredundant-decls -Waggregate-return
CFG_MT_ERROR_WARN_OPTION = -Werror=incompatible-pointer-types -Werror=uninitialized -Werror=int-to-pointer-cast -Werror=pointer-to-int-cast -Werror=int-conversion -Werror=missing-parameter-type -Werror=implicit-function-declaration -Werror=builtin-declaration-mismatch -Werror=aggressive-loop-optimizations
CFG_MT_NO_WARN_OPTION = -Wno-unused-result -Wno-unused-parameter -Wno-inline -Wno-pointer-arith -Wno-stringop-overflow -Wno-stringop-truncation -Wno-discarded-qualifiers -Wno-array-bounds -Wno-maybe-uninitialized
CFG_MT_WARN_OPTION = $(CFG_MT_YES_WARN_OPTION) $(CFG_MT_ERROR_WARN_OPTION) $(CFG_MT_NO_WARN_OPTION)

CFG_MT_CFLAGS += $(CFG_MT_WARN_OPTION)
CFG_MT_KMOD_CFLAGS += -Werror -Wno-unused-result

# CONFIG_MT_ARCH_MIPS and CONFIG_MT_ARCH_ARM and CONFIG_MT_ARCH_AARCH64 meaning user space mode
# CONFIG_MT_KERNEL_ARCH_MIPS and CONFIG_MT_KERNEL_ARCH_ARM and CONFIG_MT_KERNEL_ARCH_AARCH64 meaning kernel space mode
# CONFIG_MIPS CONFIG_ARM CONFIG_ARM64 CONFIG_AARCH64 meaning user space mode in userspace code
# but in kernel space code, CONFIG_MIPS CONFIG_ARM CONFIG_ARM64(no CONFIG_AARCH64) meaning kernel space mode
ifeq ($(CONFIG_MT_ARCH_MIPS),y)
CFG_MT_CFLAGS += -DCONFIG_MIPS
else ifeq ($(CONFIG_MT_ARCH_ARM),y)
CFG_MT_CFLAGS += -DCONFIG_ARM
else ifeq ($(CONFIG_MT_ARCH_AARCH64),y)
CFG_MT_CFLAGS += -DCONFIG_ARM64 -DCONFIG_AARCH64
endif

ifneq ($(CONFIG_MT_LOG),y)
CFG_MT_CFLAGS += -DMT_LOG_SUPPORT=0
CFG_MT_KMOD_CFLAGS += -DMT_LOG_SUPPORT=0
endif

ifeq ($(CFG_MT_CHIP),aria)
CFG_MT_CFLAGS += -DMT_VDEC_VPU_SUPPORT=1
endif

ifeq ($(CONFIG_MT_PROC_SUPPORT),y)
CFG_MT_CFLAGS += -DMT_PROC_SUPPORT=1
CFG_MT_KMOD_CFLAGS += -DMT_PROC_SUPPORT=1
endif

ifeq ($(CFG_FFMPEG_VER_NUM),422)
CFG_MT_CFLAGS += -DCFG_ENABLE_FFMPEG_422
endif

ifeq ($(CONFIG_MT_SMART_HTTP_PTOTOCOL),y)
CFG_MT_CFLAGS += -DCFG_SMART_HTTP_PTOTOCOL
endif

ifeq ($(CFG_MT_BUILD_LOADER),y)
CFG_MT_KMOD_CFLAGS += -DMT_BUILD_LOADER
CFG_MT_CFLAGS += -DMT_BUILD_LOADER
endif

ifeq ($(CONFIG_MT_PRODUCT_LOADER),y)
export CONFIG_MT_PRODUCT_LOADER
endif

ifeq ($(CFG_MT_SDK_RELEASE),y)
CFG_MT_KMOD_CFLAGS += -DCFG_MT_SDK_RELEASE
CFG_MT_CFLAGS += -DCFG_MT_SDK_RELEASE
endif



#CONFIG_EMU, do we use ?
ifeq ($(CONFIG_EMU),y)
CFG_MT_KMOD_CFLAGS += -DCONFIG_EMU=1
endif

ifeq ($(CONFIG_MT_DRM_TEE_SUPPORT),y)
CFG_MT_CFLAGS += -DMT_DRM_SUPPORT -DDRM_SMP_ENABLE
endif

CFG_MT_CFLAGS += -include $(INCLUDE_DIR)/autoconf-old.h
CFG_MT_KMOD_CFLAGS += -include $(INCLUDE_DIR)/autoconf-old.h



export INCLUDE_DIR COMMON_UNF_INCLUDE COMMON_API_INCLUDE COMMON_DRV_INCLUDE MSP_UNF_INCLUDE MSP_API_INCLUDE MSP_DRV_INCLUDE
export BUILDROOT_DIR BUILDROOT_HOST_DIR
export MFRS_CFG_MENUCONFIG_PATH
export CFG_MT_KMOD_CFLAGS
export MFRS
export AV_BIN_DIR
export MT_KERNEL_DIR MT_KERNEL_OUTPUT_OPT

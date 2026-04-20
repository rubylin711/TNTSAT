#######################################################################
#
#   Makefile for arm-mont-linux
#
#######################################################################
TARGET = arm-mont-linux
GCC_CROSS_PREFIX=/usr/local/linaro/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
AP4_PLATFORM_BYTE_ORDER=AP4_PLATFORM_BYTE_ORDER_LITTLE_ENDIAN
include ../../any-gnu-gcc/Local.mak

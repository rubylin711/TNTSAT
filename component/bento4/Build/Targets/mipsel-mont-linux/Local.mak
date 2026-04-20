#######################################################################
#
#   Makefile for mipsel-sigma-linux
#
#######################################################################
TARGET = mipsel-mont-linux
GCC_CROSS_PREFIX=/usr/local/codesourcery/mips-2016.05/bin/mips-linux-gnu-
AP4_PLATFORM_BYTE_ORDER=AP4_PLATFORM_BYTE_ORDER_LITTLE_ENDIAN
include ../../any-gnu-gcc/Local.mak

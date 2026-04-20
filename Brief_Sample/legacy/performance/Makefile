CFG_MT_BACKTRACE_ENABLE = n
CFG_MT_SSP_ALL_ENABLE = n
CFG_MT_SSP_STRONG_ENABLE = n
CFG_MT_SSP_NONE_ENABLE = y
include ${SDK_DIR}/build/script/base.mk

SUBDIRS = dhrystone pi_css5/pi_css5_src nbench stream whetstone linpack coremark off-cpu memtester
#SUBDIRS += iozone lmbench

include ${SDK_DIR}/build/script/Makefile-subdirs.rule

include ${SDK_DIR}/build/script/base.mk

OBJS = read_avs_data.o

APP = read_avs_data

INSTALL_EXTRA_SCRIPT = adjust-voltage.sh benchmark.sh benchmark-percpu.sh hotplug.sh power.sh power-percpu.sh linpack-orig.sh linpack.sh max-power-consumption.sh max-power-consumption-percpu.sh cpufreq_limit.sh avs-stable.sh dvfs-long-term-test.sh
INSTALL_EXTRA_RESOURCE = 1.jpg 2.jpg 1.jpg-x86.result 2.jpg-x86.result pi4194304-x86.txt autorun-symphony4.txt autorun-symphony6.txt

ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
endif

include ${SDK_DIR}/build/script/Makefile-app.rule

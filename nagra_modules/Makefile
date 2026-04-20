include $(SDK_DIR)/build/script/base.mk

.PHONY: all clean install uninstall

all:
	make -C MT_NOCSAPIs 
	make -C MT_NOCSAPIs install
	make -C MT_NOCSAPIs clean
	make -C MT_NOCSAPIs  -f Makefile_Talts
	make -C MT_NOCSAPIs install
	make -C MT_NOCSAPIs clean
	make -C nagra_dal
	#make -C nagra_pre_integraton_test
clean:
	make -C MT_NOCSAPIs clean
	make -C nagra_dal clean
	#make -C nagra_pre_integraton_test clean
	
install:
	make -C MT_NOCSAPIs install
	make -C nagra_dal install
	#make -C nagra_pre_integraton_test install

uninstall:
	make -C MT_NOCSAPIs uninstall
	make -C nagra_dal uninstall 
	#make -C nagra_pre_integraton_test uninstall	
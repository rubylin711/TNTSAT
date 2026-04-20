include ${SDK_DIR}/build/script/base.mk

.PHONY: all clean install uninstall

all:
	$(MAKE) -f app_nvta_master.mk
	$(MAKE) -f app_nvta_master.mk install
	$(MAKE) -f app_nvta_master.mk clean
	$(MAKE) -f app_master.mk
	$(MAKE) -f app_master.mk install	
	$(MAKE) -f app_master.mk clean	
	$(MAKE) -f app_worker.mk
	$(MAKE) -f app_worker.mk install	
	$(MAKE) -f app_worker.mk clean	

clean:
	$(MAKE) -f app_nvta_master.mk clean
	$(MAKE) -f app_master.mk clean
	$(MAKE) -f app_worker.mk clean

install:
	$(MAKE) -f app_nvta_master.mk install
	$(MAKE) -f app_master.mk install
	$(MAKE) -f app_worker.mk install

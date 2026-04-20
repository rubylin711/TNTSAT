include ${SDK_DIR}/build/script/base.mk

.PHONY: all clean install uninstall

all:
	$(MAKE) -f caller.mk
	$(MAKE) -f caller_mp1.mk
	$(MAKE) -f caller_mp2.mk
	$(MAKE) -f caller_mp3.mk
	$(MAKE) -f caller_mpfs.mk
	$(MAKE) -f listener1.mk
	$(MAKE) -f listener2.mk
	$(MAKE) -f listener3.mk
	$(MAKE) -f listener_mp.mk
	$(MAKE) -f listener_mpfs.mk
	$(MAKE) -f stress-client.mk
	$(MAKE) -f stress-server.mk
	$(MAKE) -f usb-storage-daemon.mk


clean:
	$(MAKE) -f caller.mk clean
	$(MAKE) -f caller_mp1.mk clean
	$(MAKE) -f caller_mp2.mk clean
	$(MAKE) -f caller_mp3.mk clean
	$(MAKE) -f caller_mpfs.mk clean
	$(MAKE) -f listener1.mk clean
	$(MAKE) -f listener2.mk clean
	$(MAKE) -f listener3.mk clean
	$(MAKE) -f listener_mp.mk clean
	$(MAKE) -f listener_mpfs.mk clean
	$(MAKE) -f stress-client.mk clean
	$(MAKE) -f stress-server.mk clean
	$(MAKE) -f usb-storage-daemon.mk clean


install:
	$(MAKE) -f caller.mk install
	$(MAKE) -f caller_mp1.mk install
	$(MAKE) -f caller_mp2.mk install
	$(MAKE) -f caller_mp3.mk install
	$(MAKE) -f caller_mpfs.mk install
	$(MAKE) -f listener1.mk install
	$(MAKE) -f listener2.mk install
	$(MAKE) -f listener3.mk install
	$(MAKE) -f listener_mp.mk install
	$(MAKE) -f listener_mpfs.mk install
	$(MAKE) -f stress-client.mk install
	$(MAKE) -f stress-server.mk install
	$(MAKE) -f usb-storage-daemon.mk install


uninstall:
	$(MAKE) -f caller.mk uninstall
	$(MAKE) -f caller_mp1.mk uninstall
	$(MAKE) -f caller_mp2.mk uninstall
	$(MAKE) -f caller_mp3.mk uninstall
	$(MAKE) -f caller_mpfs.mk uninstall
	$(MAKE) -f listener1.mk uninstall
	$(MAKE) -f listener2.mk uninstall
	$(MAKE) -f listener3.mk uninstall
	$(MAKE) -f listener_mp.mk uninstall
	$(MAKE) -f listener_mpfs.mk uninstall
	$(MAKE) -f stress-client.mk uninstall
	$(MAKE) -f stress-server.mk uninstall
	$(MAKE) -f usb-storage-daemon.mk uninstall


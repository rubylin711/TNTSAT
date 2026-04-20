cp Kconfig.$1 Kconfig.rule
make
if [ "$1" = "sdio" ]; then
	cp m88wi6700s.ko ../util/mont_ctl/
fi

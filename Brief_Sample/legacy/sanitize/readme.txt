make flash; cp -af pub/bin_striped/test_sanitize pub/rootfs_striped/usr/bin; cp -af pub/shared_lib_striped/libtest_sanitize.so pub/rootfs_striped/usr/lib; make flash; make nfs



make clean; make; arm-linux-gnueabihf-objdump -hD -C /home/lujiang/work/mt_sdk/symphony/linux/out/general/sample/sanitize/test_sanitize > app.asm; arm-linux-gnueabihf-objdump -hD -C /home/lujiang/work/mt_sdk/symphony/linux/out/general/sample/sanitize/libtest_sanitize.so > lib.asm

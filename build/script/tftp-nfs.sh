#!/bin/bash

ip_addr=${MT_IP_ADDR}
echo -e "\033[32m ip_addr=${ip_addr} \033[0m"

if [ "rcS" = "$1" ]; then
	echo "set rcS nfs"
	sed -i 's|your_compile_server_ip|'${ip_addr}'|g' ${SDK_DIR}/image/${MFRS}/rootfs/etc/init.d/S09mount
	sed -i 's|your_linux_sdk_image_mfrs_path|'${SDK_DIR}/image/${MFRS}'|g' ${SDK_DIR}/image/${MFRS}/rootfs/etc/init.d/S09mount
fi

if [ "env" = "$1" ]; then
	echo "set env nfs"
	sed -i 's|your_compile_server_ip|'${ip_addr}'|g' $2
	sed -i 's|your_linux_sdk_image_mfrs_path|'${SDK_DIR}/image/${MFRS}'|g' $2
	sed -i 's|AV_BIN_TFTP_DIR|'${AV_BIN_TFTP_DIR}'|g' $2
	sed -i 's|KERNEL_BIN_TFTP_DIR|'${KERNEL_BIN_TFTP_DIR}'|g' $2
fi

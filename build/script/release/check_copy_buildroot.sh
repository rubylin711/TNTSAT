#!/bin/bash



if [ "${JENKINS_COMPILE_CHECK}" != "jenkins" ]; then
	exit 0
fi


#workspace/.repo/manifests/ workspace/ddk/kernel/linux-x.y.z/ workspace/ddk/msp/
#SDK_DIR at workspace/ddk/
buildroot_output="${SDK_DIR}/../../Lznux-buildroot-output-backup"
mkdir -p ${buildroot_output}
grep "CONFIG_MT_BUILDROOT_DEFCONFIG" ${MFRS_CFG_MENUCONFIG_PATH}/.config > ${buildroot_output}/buildroot-commit-new.txt
cd ${BUILDROOT_DIR}; git log -1 >> ${buildroot_output}/buildroot-commit-new.txt; cd -



if [ $1 = "step0" ]; then
	if [ -f "${buildroot_output}/buildroot-commit-old.txt" ]; then
		if [ -n "`diff ${buildroot_output}/buildroot-commit-new.txt ${buildroot_output}/buildroot-commit-old.txt`" ]; then
			echo ""
			echo -e "\033[35m diff, compile new buildroot \033[0m"
			echo ""
			copy_buildroot_output="no"
		else
			echo ""
			echo -e "\033[35m same, copy old buildroot \033[0m"
			echo ""
			copy_buildroot_output="yes"
		fi
	else
		echo ""
		echo -e "\033[35m not found old, compile new buildroot \033[0m"
		echo ""
		copy_buildroot_output="no"
	fi

	if [ "${copy_buildroot_output}" = "yes" ]; then
		if [ -d "${SDK_DIR}/buildroot" ]; then
			rm -rf ${SDK_DIR}/buildroot/output
			cp -arf ${buildroot_output}/output ${SDK_DIR}/buildroot/
		else
			echo ""
			echo -e "\033[35m not found buildroot directory, can't copy \033[0m"
			echo ""
			exit 1
		fi
	fi
fi



if [ $1 = "step1" ]; then
	if [ -f "${buildroot_output}/buildroot-commit-old.txt" ]; then
		if [ -n "`diff ${buildroot_output}/buildroot-commit-new.txt ${buildroot_output}/buildroot-commit-old.txt`" ]; then
			echo ""
			echo -e "\033[35m diff, update backup \033[0m"
			echo ""
			update_backup="yes"
		else
			echo ""
			echo -e "\033[35m same, not update backup \033[0m"
			echo ""
			update_backup="no"
		fi
	else
		echo ""
		echo -e "\033[35m not found old, update backup \033[0m"
		echo ""
		update_backup="yes"
	fi

	if [ "${update_backup}" = "yes" ]; then
		if [ -d "${SDK_DIR}/buildroot/output" ]; then
			rm -rf ${buildroot_output}/output
			cp -arf ${SDK_DIR}/buildroot/output ${buildroot_output}
			rm -rf ${buildroot_output}/output/host/${BUILDROOT_HOST_DIR}/sysroot/usr/include/mt
			rm -rf ${buildroot_output}/output/host/${BUILDROOT_HOST_DIR}/sysroot/usr/lib/mt
			rm -rf ${buildroot_output}/output/host/${BUILDROOT_HOST_DIR}/sysroot/usr/bin/mt
			rm -rf ${buildroot_output}/output/host/${BUILDROOT_HOST_DIR}/sysroot/usr/static_lib/mt
			cp ${buildroot_output}/buildroot-commit-new.txt ${buildroot_output}/buildroot-commit-old.txt
		else
			echo ""
			echo -e "\033[35m not found buildroot/output directory, can't update, you must full compile first \033[0m"
			echo ""
			exit 1
		fi
	fi
fi

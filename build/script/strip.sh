#!/bin/bash

if [ "${STRIP_DEBUG}" = "--strip-debug" ]; then
	echo -e "\033[32m strip for perf, keep symtab \033[0m"
fi

if [ "install" = "$1" ]; then
	cd ${SHARED_LIB_DIR_STRIPED}
	ALL_FILES="`find -type f`"
	for x in ${ALL_FILES}; do
		if [ -n "`file ${x} | grep ELF`" ]; then
			y="`basename ${x}`".debug
			${OBJCOPY} --only-keep-debug ${x} ${DEBUG_INFO_DIR}/${y}
			build_id="`${READELF} -n ${DEBUG_INFO_DIR}/${y} | grep "Build ID:" | awk '{print $3}'`"
			if [ -n "${build_id}" ]; then
				build_id_dir="`echo ${build_id} | dd bs=1 count=2 2>/dev/null`"
				build_id_file="`echo ${build_id} | dd bs=1 skip=2 2>/dev/null`".debug
				mkdir -p ${DEBUG_INFO_DIR}/.build-id/${build_id_dir}
				mv ${DEBUG_INFO_DIR}/${y} ${DEBUG_INFO_DIR}/.build-id/${build_id_dir}/${build_id_file}
			else
				mv ${DEBUG_INFO_DIR}/${y} ${DEBUG_INFO_DIR}/${y}.miss-build-id
			fi

			chmod +w ${x}
			${STRIP} ${STRIP_DEBUG} ${x}
			chmod -w ${x}
			echo "${STRIP} ${STRIP_DEBUG} ${x}"
		fi
	done

	cd ${BIN_DIR_STRIPED}
	ALL_FILES="`find -type f`"
	for x in ${ALL_FILES}; do
		if [ -n "`file ${x} | grep ELF`" ]; then
			y="`basename ${x}`".debug
			${OBJCOPY} --only-keep-debug ${x} ${DEBUG_INFO_DIR}/${y}
			build_id="`${READELF} -n ${DEBUG_INFO_DIR}/${y} | grep "Build ID:" | awk '{print $3}'`"
			if [ -n "${build_id}" ]; then
				build_id_dir="`echo ${build_id} | dd bs=1 count=2 2>/dev/null`"
				build_id_file="`echo ${build_id} | dd bs=1 skip=2 2>/dev/null`".debug
				mkdir -p ${DEBUG_INFO_DIR}/.build-id/${build_id_dir}
				mv ${DEBUG_INFO_DIR}/${y} ${DEBUG_INFO_DIR}/.build-id/${build_id_dir}/${build_id_file}
			else
				mv ${DEBUG_INFO_DIR}/${y} ${DEBUG_INFO_DIR}/${y}.miss-build-id
			fi

			chmod +w ${x}
			${STRIP} ${STRIP_DEBUG} ${x}
			chmod -w ${x}
			echo "${STRIP} ${STRIP_DEBUG} ${x}"
		fi
	done

	cd ${MODULE_DIR_STRIPED}
	ALL_FILES="`find -type f`"
	for x in ${ALL_FILES}; do
		if [ -n "`file ${x} | grep ELF`" ]; then
			chmod +w ${x}
			${STRIP_KERNEL} --strip-debug ${x}
			chmod -w ${x}
			echo "${STRIP_KERNEL} --strip-debug ${x}"
		fi
	done
fi


if [ "rootfs" = "$1" ]; then
	cd ${MONTAGE_ROOTFS_STRIPED}
	ALL_FILES="`find -type f`"
	for x in ${ALL_FILES}; do
		if [ -n "`file ${x} | grep ELF`" ]; then
			y="`basename ${x}`".debug
			${OBJCOPY} --only-keep-debug ${x} ${DEBUG_INFO_DIR}/${y}
			build_id="`${READELF} -n ${DEBUG_INFO_DIR}/${y} | grep "Build ID:" | awk '{print $3}'`"
			if [ -n "${build_id}" ]; then
				build_id_dir="`echo ${build_id} | dd bs=1 count=2 2>/dev/null`"
				build_id_file="`echo ${build_id} | dd bs=1 skip=2 2>/dev/null`".debug
				mkdir -p ${DEBUG_INFO_DIR}/.build-id/${build_id_dir}
				mv ${DEBUG_INFO_DIR}/${y} ${DEBUG_INFO_DIR}/.build-id/${build_id_dir}/${build_id_file}
			else
				mv ${DEBUG_INFO_DIR}/${y} ${DEBUG_INFO_DIR}/${y}.miss-build-id
			fi

			chmod +w ${x}
			${STRIP} ${STRIP_DEBUG} ${x}
			chmod -w ${x}
			echo "${STRIP} ${STRIP_DEBUG} ${x}"
		fi
	done
fi

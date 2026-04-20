#!/bin/bash

if [ "$1" = "release" ]; then
	all="`grep -nrIl "opensource" opensource/output`"
	for x in $all;
	do
		#echo $x
		tools/linux/mk-pkg-la-path.bin $x
		sed -i "s|${SDK_DIR}|\${SDK_DIR}|g" $x
	done

#	all="`grep -nrIl "opensource-2nd" opensource-2nd/output`"
#	for x in $all;
#	do
#		#echo $x
#		tools/linux/mk-pkg-la-path.bin $x
#		sed -i "s|${SDK_DIR}|\${SDK_DIR}|g" $x
#	done
else
	all="`grep -nrIl "opensource" opensource/output`"
	for x in $all;
	do
		#echo $x
		sed -i "s|\${SDK_DIR}|${SDK_DIR}|g" $x
	done

#	all="`grep -nrIl "opensource-2nd" opensource-2nd/output`"
#	for x in $all;
#	do
#		#echo $x
#		sed -i "s|\${SDK_DIR}|${SDK_DIR}|g" $x
#	done
fi

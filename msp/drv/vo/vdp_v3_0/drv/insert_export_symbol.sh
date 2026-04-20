#!/bin/bash

# 1, run the folowing commmand manumotive to add EXPORT_SYMBOL(); after }
# 	sed -i 's|}|}\nEXPORT_SYMBOL();|g' $1
# 2, then, delete the no need EXPORT_SYMBOL(); manumotive
# 3, then, run me


LINE_CNT="`wc -l $1 | awk '{print $1}'`"
echo ${LINE_CNT}

i=1
while [ ${i} -le ${LINE_CNT} ]
do
	export_symbol="`sed -n -e ''${i}'p' $1`"
	export_symbol="`echo ${export_symbol} | grep -n EXPORT_SYMBOL`"
	if [ -n "${export_symbol}" ]; then
		j=${i}
		while true
		do
			context="`sed -n -e ''${j}'p' $1 | grep -n '{'`"
			if [ -n "${context}" ]; then
				j="`expr ${j} - 1`"
				func="`sed -n -e ''${j}'p' $1 | awk '{print $2}' | awk -F'(' '{print $1}'`"
				sed -i -e ''${i}' s|EXPORT_SYMBOL()|EXPORT_SYMBOL('${func}')|' $1
				break
			fi
			j="`expr ${j} - 1`"
		done
	fi
	i="`expr ${i} + 1`"
done

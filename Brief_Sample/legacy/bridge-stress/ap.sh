x="1";
while true; do
	echo "0x2 > /sys/kernel/debug/cache_and_bus_test"
	echo 0x2 > /sys/kernel/debug/cache_and_bus_test;

	echo "x=$x"; x="`expr $x + 1`";
	echo "x=$x" > x.txt
done

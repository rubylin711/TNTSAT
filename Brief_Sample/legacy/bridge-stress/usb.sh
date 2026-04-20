echo "dd if=/dev/zero of=/media/sda1/zero bs=1048576 count=1024"
dd if=/dev/zero of=/media/sda1/zero bs=1048576 count=1024; sync; ls -l /media/sda1/zero

z="1"
while true; do
	echo "dd if=/media/sda1/zero of=/media/sda1/dd-zero bs=1048576 count=1024"
	dd if=/media/sda1/zero of=/media/sda1/dd-zero bs=1048576 count=1024; sync; ls -l /media/sda1/zero

	echo "z=$z"; z="`expr $z + 1`";
	echo "z=$z" > z.txt
done




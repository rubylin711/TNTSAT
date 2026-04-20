#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ONLINE "/sys/devices/system/cpu/cpu1/online"

int main(int argc, char **argv)
{
	int fd;

	fd = open(ONLINE, O_RDWR, S_IRUSR | S_IWUSR);

	while (1) {
		lseek(fd, 0, SEEK_SET);
		write(fd, "0", strlen("0") + 1);
		lseek(fd, 0, SEEK_SET);
		write(fd, "1", strlen("1") + 1);
	}

	return 0;
}



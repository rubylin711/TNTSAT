/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
	char buf[4096] = {0};
	int fd;
	int print_banner_multi;
	int print_banner_first = 1;

	print_banner_multi = atoi(argv[1]);

	fd = open("/sys/kernel/debug/regulator/vcc_cpu/ringo", O_RDWR, 0644);
	while (1) {
		if (print_banner_multi || print_banner_first) {
			print_banner_first = 0;
			printf("FREQ	LOAD	TMP	VCODE	COR0	COR1	COR2	COR3	COH0	COH1	COH2	COH3	CR0	CR1	CR2	CH0	CH1	CH2\n");
		}
		lseek(fd, 0, SEEK_SET);
		read(fd, buf, sizeof(buf));
		printf("%s", buf);
		sleep(1);
	}
	return 0;
}



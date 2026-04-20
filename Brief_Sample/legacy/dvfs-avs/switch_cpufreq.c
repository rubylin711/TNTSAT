/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
	int rand;
	char *freq[100] = {NULL};
	char *p;
	int i;
	int total;
	char buffer[1024];
	int fd;

	if (argc < 2) {
		printf("switch_cpufreq random   or  switch_cpufreq sequence\n");
		return -1;
	}

	rand = !strcmp(argv[1], "random") ? 1 : 0;

	fd = open("/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies", O_RDONLY);
	read(fd, buffer, sizeof(buffer));
	close(fd);
	printf("%s\n", buffer);

	p = strtok(buffer, " ");
	i = 0;
	while (p != NULL && (*p >= '0' && *p <= '9')) {
		freq[i] = p;
		printf("freq[%d] = %s\n", i, freq[i]);
		i++;
		if (i >= sizeof(freq) / sizeof(freq[0])) {
			printf("too many freq point\n");
			return -1;
		}
		p = strtok(NULL, " ");
	};
	total = i;

	fd = open("/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed", O_RDWR, 0644);
	while (1) {
		for (i = 0; i < total; i++) {
			if (rand == 0) {
				p = freq[i];
			} else {
				p = freq[(random() % total)];
			}
			printf("%s\n", p);
			if (p != NULL) {
				write(fd, p, strlen(p) + 1);
				system("cat /sys/kernel/debug/clk/cpu/cpu_clk_index | grep cur_index");
			} else {
				printf("p is NULL\n");
			}
		}
		printf("\n");
	}

	return 0;

}

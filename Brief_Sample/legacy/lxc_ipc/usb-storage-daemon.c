/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include "lxc_ipc.h"

static char uevent[8192];

int main(int argc, char **argv)
{
	struct sockaddr_nl snl;
	int snl_fd;
	ssize_t len;
	char *p;
	char disk_partition[32];
	char comm[256];

	memset(&snl, 0x00, sizeof(struct sockaddr_nl));
	snl.nl_family = AF_NETLINK;
	snl.nl_pid = (__u32)getpid();
	snl.nl_groups = 1; /* must be 1, because kernel set it as 1 when create NETLINK_KOBJECT_UEVENT */

	//daemon_init();

	snl_fd = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (snl_fd == -1) {
		printf("socket failed\n");
		return 1;
	}

	bind(snl_fd, (void *)&snl, sizeof(snl));

	while (1) {
		len = recv(snl_fd, uevent, sizeof(uevent), 0);
		if (len == -1) {
			printf("recv failed\n");
		} else if (len == 0) {
			break;
		} else {
			printf("%s\n\n", uevent);
		}

		if (strstr(uevent, "block/sd")) {
			if (strstr(uevent, "add@")) {
				p = strstr(uevent, "block/sd");
				p += strlen("block/sd");
				if (*(p + 1) == '\0') {
					snprintf(disk_partition, sizeof(disk_partition), "sd%c", *p);
					snprintf(comm, sizeof(comm), "mkdir -p /media/%s", disk_partition);
					printf("\33[44;31m %s \33[0m\n", comm);
					system(comm);
				} else if (*(p + 1) == '/') {
					snprintf(disk_partition, sizeof(disk_partition), "sd%c%s", *p, p + 5);
					snprintf(comm, sizeof(comm), "mkdir -p /media/%s", disk_partition);
					printf("\33[44;31m %s \33[0m\n", comm);
					system(comm);
					snprintf(comm, sizeof(comm), "mount /dev/%s /media/%s", disk_partition, disk_partition);
					printf("\33[44;31m %s \33[0m\n", comm);
					system(comm);
				}
			} else if (strstr(uevent, "remove@")) {
				p = strstr(uevent, "block/sd");
				p += strlen("block/sd");
				if (*(p + 1) == '\0') {
					/* don't rmdir /media/sda */
				} else if (*(p + 1) == '/') {
					snprintf(disk_partition, sizeof(disk_partition), "sd%c%s", *p, p + 5);
					snprintf(comm, sizeof(comm), "umount /media/%s", disk_partition);
					printf("\33[44;31m %s \33[0m\n", comm);
					system(comm);
					/* don't rmdir /media/sda1 */
				}
			}
		}
	}

	return 0;
}

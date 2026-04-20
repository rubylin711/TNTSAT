/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/genetlink.h>

#define 	JILL_BUFFER_SIZE 		16
#define 	JILL_SET 		(_IOWR('J', 1, int))
#define 	JILL_CLEAR 		(_IOWR('J', 2, int))
#define	JILL_MMAP_SIZE		(8192)
#define	JILL_MMAP_OFF		(0)
#define	JILL_NL_NAME		"jill_nl"
#define	JILL_SEQ			0x11223344

/* struct and enum and marco define all be copied from drivers/jill/jill.c */

/* cmd0 can match data0 and data1, cmd1 also can match data0 and data1 */
enum {
	JILL_NL_OPS_CMD0,
	JILL_NL_OPS_CMD1,
	__JILL_NL_OPS_MAX,
};
#define	JILL_NL_OPS_AMOUNT			(__JILL_NL_OPS_MAX)

enum {
	JILL_NL_ATTR_UNSPEC,
	JILL_NL_ATTR_DATA0,
	JILL_NL_ATTR_DATA1,
	__JILL_NL_ATTR_MAX,
};
#define	JILL_NL_FAMILY_ATTR_MAX		(__JILL_NL_ATTR_MAX - 1)
#define	JILL_NL_FAMILY_ATTR_AMOUNT	(__JILL_NL_ATTR_MAX)

struct jill_netlink_data {
	pid_t pid;
	unsigned int seq;
	unsigned int x;
	unsigned int y;
	unsigned int data01;
	unsigned char cmd;
	char name[32];
};

int finish = 0;

static void *netlink_thread(void *arg)
{
	int sock_fd;
	struct sockaddr_nl app_addr;
	struct sockaddr_nl kernel_addr;
	struct msghdr msg;
	struct nlmsghdr *nlh = NULL;
	struct nlmsghdr *nlh_family = NULL;
	struct nlmsghdr *nlh_snd = NULL;
	struct nlmsghdr *nlh_rcv = NULL;
	struct genlmsghdr *gehdr = NULL;
	struct nlattr *nla = NULL;
	void *pos = NULL;
	struct iovec iov[1];
	struct jill_netlink_data *jill_nl_data = NULL;
	int len_family;
	int len_snd;
	int len_rcv;
	int sendcnt;
	int recvcnt;
	char *jill_nl_name = NULL;
	int jill_nl_name_len;
	int rem;
	int family_id;
	int seed = 0;

	sock_fd = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_GENERIC);

	memset(&app_addr, 0, sizeof(app_addr));
	app_addr.nl_family = AF_NETLINK;
	app_addr.nl_pid = getpid(); /* self pid */
	app_addr.nl_groups = 0;

	printf("app_addr.nl_pid = %d\n", app_addr.nl_pid);

	bind(sock_fd, (struct sockaddr*)&app_addr, sizeof(app_addr));

	len_family = NLMSG_SPACE(GENL_HDRLEN + NLA_HDRLEN + GENL_NAMSIZ);
	nlh_family = (struct nlmsghdr *)malloc(len_family);
	nlh = nlh_family;
	memset(nlh, 0, len_family);

	nlh->nlmsg_len = len_family;
	nlh->nlmsg_pid = (pthread_self() << 16 | getpid()); /* self id */
	nlh->nlmsg_flags = NLM_F_REQUEST;
	nlh->nlmsg_type = GENL_ID_CTRL;
	nlh->nlmsg_seq = JILL_SEQ;

	gehdr = NLMSG_DATA(nlh);
	gehdr->cmd = CTRL_CMD_GETFAMILY;		/* copy from genl_ctrl_ops in kernel */
	gehdr->version = 0x2;
	gehdr->reserved = 0;

	nla = (void *)((void *)gehdr + GENL_HDRLEN);
	jill_nl_name_len = strlen(JILL_NL_NAME);
	nla->nla_type = CTRL_ATTR_FAMILY_NAME;
	nla->nla_len = NLA_HDRLEN + NLA_ALIGN(jill_nl_name_len + 1);
	jill_nl_name = (void *)((void *)nla + NLA_HDRLEN);
	strcpy(jill_nl_name, JILL_NL_NAME);
	jill_nl_name[jill_nl_name_len] = '\0';

	iov[0].iov_base = (void *)nlh;
	iov[0].iov_len = nlh->nlmsg_len;

	memset(&kernel_addr, 0, sizeof(kernel_addr));
	kernel_addr.nl_family = AF_NETLINK;
	kernel_addr.nl_pid = 0; /* send to kernel */
	kernel_addr.nl_groups = 0;

	memset(&msg, 0, sizeof(msg));
	msg.msg_name = (void *)&kernel_addr;
	msg.msg_namelen = sizeof(kernel_addr);
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	sendcnt = sendmsg(sock_fd, &msg, 0);		/* ask jill family id */
	printf("sendcnt ctrl = %d\n", sendcnt);
	free(nlh_family);


	len_family = 4096;		/* from NLMSG_DEFAULT_SIZE + NLMSG_HDRLEN */
	nlh_family = (struct nlmsghdr *)malloc(len_family);
	memset(nlh_family, 0, len_family);

	memset(&msg, 0, sizeof(msg));
	iov[0].iov_base = (void *)nlh_family;
	iov[0].iov_len = len_family;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	recvcnt = recvmsg(sock_fd, &msg, 0);
	if (recvcnt <= 0) {
		printf("recv family id failed\n");
		return NULL;
	}
	printf("recvcnt ctrl = %d\n", recvcnt);

	nlh = nlh_family;
	while (NLMSG_OK(nlh, recvcnt)) {
		printf("nlh->nlmsg_len = %d\n", nlh->nlmsg_len);
		printf("nlh->nlmsg_seq = 0x%x\n", nlh->nlmsg_seq);

		if (nlh->nlmsg_type != GENL_ID_CTRL) {
			nlh = NLMSG_NEXT(nlh, recvcnt);
			continue;
		}

		if ((recvcnt - GENL_HDRLEN) >= (int)sizeof(struct nlmsghdr))
			gehdr = NLMSG_DATA(nlh);
		else {
			nlh = NLMSG_NEXT(nlh, recvcnt);
			continue;
		}
		nla = (void *)((void *)gehdr + GENL_HDRLEN);

		for (pos = nla, rem = nlh->nlmsg_len - NLMSG_HDRLEN - GENL_HDRLEN;
			rem >= NLA_HDRLEN && nla->nla_len >= NLA_HDRLEN && nla->nla_len <= rem;
			rem -= NLA_ALIGN(nla->nla_len), pos += NLA_ALIGN(nla->nla_len)) {
			if (((struct nlattr *)pos)->nla_len > NLA_HDRLEN) {
				if (((struct nlattr *)pos)->nla_type == CTRL_ATTR_FAMILY_ID) {
					family_id = *(int *)((void *)pos + NLA_HDRLEN);
					printf("family_id = %d\n", family_id);
					free(nlh_family);
					goto got_jill_family_id;
				}
			}
		}
		nlh = NLMSG_NEXT(nlh, recvcnt);
	}
	printf("not found family id\n");
	return NULL;



got_jill_family_id:		/* now communicate with jill */

	len_snd = NLMSG_SPACE(GENL_HDRLEN + NLA_HDRLEN + sizeof(struct jill_netlink_data));
	nlh_snd = (struct nlmsghdr *)malloc(len_snd);
	memset(nlh_snd, 0, len_snd);
	nlh = nlh_snd;

	len_rcv = 4096;		/* from NLMSG_DEFAULT_SIZE + NLMSG_HDRLEN */
	nlh_rcv = (struct nlmsghdr *)malloc(len_rcv);
	memset(nlh_rcv, 0, len_rcv);

	while (!finish) {
		nlh = nlh_snd;
		memset(nlh, 0, len_snd);

		nlh->nlmsg_len = len_snd;
		nlh->nlmsg_pid = (pthread_self() << 16 | getpid()); /* self id */
		nlh->nlmsg_seq = JILL_SEQ + seed;
		nlh->nlmsg_flags = NLM_F_REQUEST;
		nlh->nlmsg_type = family_id;

		gehdr = NLMSG_DATA(nlh);
		gehdr->version = 0x1;
		gehdr->reserved = 0;

		nla = (void *)((void *)gehdr + GENL_HDRLEN);
		nla->nla_len = NLA_HDRLEN + NLA_ALIGN(sizeof(struct jill_netlink_data));
		jill_nl_data = (void *)((void *)nla + NLA_HDRLEN);
		snprintf(jill_nl_data->name, sizeof(jill_nl_data->name), "test_jill.elf_%d", seed);
		jill_nl_data->pid = getpid();
		jill_nl_data->seq = nlh->nlmsg_seq;
		jill_nl_data->x = 7;
		jill_nl_data->y = 9;

		switch (seed % 4) {
		case 0:
			gehdr->cmd = JILL_NL_OPS_CMD0;
			nla->nla_type = JILL_NL_ATTR_DATA0;
			break;
		case 1:
			gehdr->cmd = JILL_NL_OPS_CMD0;
			nla->nla_type = JILL_NL_ATTR_DATA1;
			break;
		case 2:
			gehdr->cmd = JILL_NL_OPS_CMD1;
			nla->nla_type = JILL_NL_ATTR_DATA0;
			break;
		default:		/* 3 */
			gehdr->cmd = JILL_NL_OPS_CMD1;
			nla->nla_type = JILL_NL_ATTR_DATA1;
		}
		jill_nl_data->cmd = gehdr->cmd;
		jill_nl_data->data01 = nla->nla_type;

		iov[0].iov_base = (void *)nlh;
		iov[0].iov_len = nlh->nlmsg_len;

		memset(&kernel_addr, 0, sizeof(kernel_addr));
		kernel_addr.nl_family = AF_NETLINK;
		kernel_addr.nl_pid = 0; /* send to kernel */
		kernel_addr.nl_groups = 0;

		memset(&msg, 0, sizeof(msg));
		printf("len_snd = %d\n", len_snd);
		msg.msg_name = (void *)&kernel_addr;
		msg.msg_namelen = sizeof(kernel_addr);
		msg.msg_iov = iov;
		msg.msg_iovlen = 1;
		sendcnt = sendmsg(sock_fd, &msg, 0);
		printf("sendcnt jill = %d\n", sendcnt);

		seed++;



		memset(&msg, 0, sizeof(msg));
		iov[0].iov_base = (void *)nlh_rcv;
		iov[0].iov_len = len_rcv;
		msg.msg_iov = iov;
		msg.msg_iovlen = 1;
read_again:
		if (finish)
			return NULL;
		recvcnt = recvmsg(sock_fd, &msg, MSG_DONTWAIT);
		if (recvcnt <= 0)
			goto read_again;
		printf("recvcnt jill = %d\n", recvcnt);

		nlh = nlh_rcv;
		while (NLMSG_OK(nlh, recvcnt)) {
			if ((recvcnt - GENL_HDRLEN) >= (int)sizeof(struct nlmsghdr))
				gehdr = NLMSG_DATA(nlh);
			else {
				nlh = NLMSG_NEXT(nlh, recvcnt);
				continue;
			}
			nla = (void *)((void *)gehdr + GENL_HDRLEN);

			for (pos = nla, rem = nlh->nlmsg_len - NLMSG_HDRLEN - GENL_HDRLEN;
				rem >= NLA_HDRLEN && nla->nla_len >= NLA_HDRLEN && nla->nla_len <= rem;
				rem -= NLA_ALIGN(nla->nla_len), pos += NLA_ALIGN(nla->nla_len)) {
				if (((struct nlattr *)pos)->nla_len > NLA_HDRLEN) {
					if (((struct nlattr *)pos)->nla_type == JILL_NL_ATTR_DATA0 || ((struct nlattr *)pos)->nla_type == JILL_NL_ATTR_DATA1) {
						jill_nl_data = (void *)(pos + NLA_HDRLEN);
						printf("jill : pid = %d\n", jill_nl_data->pid);
						printf("jill : seq = %d\n", jill_nl_data->seq);
						printf("jill : cmd = %d\n", jill_nl_data->cmd);
						printf("jill : data01 = %d\n", jill_nl_data->data01);
						printf("jill : name : %s, x = %u, y = %u\n", jill_nl_data->name, jill_nl_data->x, jill_nl_data->y);
					}
				}
			}
			nlh = NLMSG_NEXT(nlh, recvcnt);
		}
	}
	close(sock_fd);
	free(nlh_snd);
	free(nlh_rcv);
	printf("netlink_thread finished\n");
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t thread;
	void *pvPtr = NULL;
	int iFd = -1;
	unsigned int i;
	unsigned int idx;
	unsigned int uiArg;
	unsigned char aucBuffer[JILL_BUFFER_SIZE];
	char device_node_name[] = "/dev/jill0";
	char i8C;

	printf("main pid = %d\n", getpid());

	printf("	aucBuffer: %p\n", aucBuffer);

	if (argc < 2) {
		printf("./test_jill 0 or 1\n");
		return 0;
	}

	idx = atoi(argv[1]);
	device_node_name[strlen(device_node_name) - 1] = '0' + idx;

	for (i = 0; i < JILL_BUFFER_SIZE; i++)
	{
		aucBuffer[i] = i;
	}

	iFd = open(device_node_name, O_RDWR);
	if (iFd < 0)
	{
		return 0;
	}

	printf("	write aucBuffer to driver\n");
	printf("	aucBuffer[0] = %d\n", aucBuffer[0]);
	printf("	aucBuffer[1] = %d\n", aucBuffer[1]);
	printf("	aucBuffer[%d] = %d\n", JILL_BUFFER_SIZE - 2, aucBuffer[JILL_BUFFER_SIZE - 2]);
	printf("	aucBuffer[%d] = %d\n", JILL_BUFFER_SIZE - 1, aucBuffer[JILL_BUFFER_SIZE - 1]);
	write(iFd, aucBuffer, JILL_BUFFER_SIZE);

	lseek(iFd, 0, SEEK_SET);

	memset(aucBuffer, 0, JILL_BUFFER_SIZE);
	printf("	memset aucBuffer to 0\n");
	printf("	aucBuffer[0] = %d\n", aucBuffer[0]);
	printf("	aucBuffer[1] = %d\n", aucBuffer[1]);
	printf("	aucBuffer[%d] = %d\n", JILL_BUFFER_SIZE - 2, aucBuffer[JILL_BUFFER_SIZE - 2]);
	printf("	aucBuffer[%d] = %d\n", JILL_BUFFER_SIZE - 1, aucBuffer[JILL_BUFFER_SIZE - 1]);

	read(iFd, aucBuffer, JILL_BUFFER_SIZE);
	printf("	read aucBuffer from driver\n");
	printf("	aucBuffer[0] = %d\n", aucBuffer[0]);
	printf("	aucBuffer[1] = %d\n", aucBuffer[1]);
	printf("	aucBuffer[%d] = %d\n", JILL_BUFFER_SIZE - 2, aucBuffer[JILL_BUFFER_SIZE - 2]);
	printf("	aucBuffer[%d] = %d\n", JILL_BUFFER_SIZE - 1, aucBuffer[JILL_BUFFER_SIZE - 1]);

	pvPtr = mmap(NULL, JILL_MMAP_SIZE, PROT_WRITE|PROT_READ, MAP_SHARED, iFd, JILL_MMAP_OFF);
	printf("	mmap:	pvPtr = %p\n", pvPtr);
	printf("	mmap:	pvPtr : %s\n", (char *)pvPtr);

	finish = 0;
	pthread_create(&thread, NULL, netlink_thread, NULL);

	while (1)
	{
		printf("Press key...\n");
		i8C = getchar();
		switch (i8C)
		{
			case 'c':
				ioctl(iFd, JILL_CLEAR);
				break;

			case 's':
				uiArg = 5;
				ioctl(iFd, JILL_SET, &uiArg);
				break;

			case 'w':
				lseek(iFd, 0, SEEK_SET);
				write(iFd, aucBuffer, JILL_BUFFER_SIZE);
				break;

			case 'q':
				close(iFd);
				finish = 1;
				goto exit;

			default:
				break;

		}
	}

exit:
	pthread_join(thread, NULL);
	printf("test finish\n");

	return 1;
}


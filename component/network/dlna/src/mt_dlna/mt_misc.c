#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <asm/ioctls.h>
#include <net/if.h>  
#include <net/if_arp.h>  
#include <arpa/inet.h>  
#include <sys/ioctl.h>  
#include <errno.h>  
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "mt_cdlna.h"

static int __mt_cmdresult(char* cmd, char* buf, int len)
{
	FILE *fp;
	int rlen;

	fp = popen(cmd, "r");
	if (!fp)
		return len;

	rlen = fread(buf, 1, len, fp);
	pclose(fp);
	return rlen;
}

static unsigned __get_hexval(char* buf)
{
	unsigned val = 0;
	char ch;

	if (buf[0] == '0' && buf[1] == 'x')
		buf += 2;

	while (*buf) {
		ch = *buf++;
		if (ch >= '0' && ch <= '9')
			ch -= '0';
		else if (ch >= 'A' && ch <= 'F')
			ch = ch - 'A' + 10;
		else if (ch >= 'a' && ch <= 'f')
			ch = ch - 'a' + 10;
		else
			break;
		val = (val << 4) + ch;

	}

	return val;

}

static int __mt_getnetif(char* ifname, unsigned flag)
{
	char cmd[128];
	char result[64];
	char result1[64];
	int len, rlen;
	char *p1, *p2;

	sprintf(cmd, "ls /sys/class/net");
	len = __mt_cmdresult(cmd, result, sizeof(result));
	if (len >= sizeof(result))
		return 0;

	p1 = result;
	while (1) {
		p2 = strstr(p1, "\n");
		if (!p2)
			break;
		*p2 = 0;
		sprintf(cmd, "cat /sys/class/net/%s/flags", p1);
		printf("\ncmd = %s", cmd);	
		len =  __mt_cmdresult(cmd, result1, sizeof(result));
		if (len >= sizeof(result1))
			break;
		result1[len] = 0;
		rlen = __get_hexval(result1);
		if ((rlen & flag) == flag) {
			strcpy(ifname, p1);
			return 1;
		} else
			printf("\nFlas = %X", rlen);
		p1 = p2 + 1;
		
	}	

	return 0;

}

unsigned mt_get_ipaddr(void)
{
	char ifname[17];
	int sk;
	struct sockaddr_in *sin;
	struct ifreq ifr;

	if (!__mt_getnetif(ifname, 0x1003))
		return 0;

	if ((sk = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket Error\t");
		return 0;
	}	
	
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name) - 1);
	
	if (ioctl(sk, SIOCGIFADDR, &ifr) < 0) {
		perror("Ioctl\t");
		close(sk);
		return 0;
	}

	sin = (struct sockaddr_in*)&ifr.ifr_addr;
	close(sk);
	return sin->sin_addr.s_addr;

}


#ifdef MISCTEST

int main(int argc, char* argv[])
{
	unsigned ip;
	void* f_filter;
	void* dirs = NULL;
	
	ip = get_ipaddr();
	printf("\nIP Addr = %X <%s>\n", ip, inet_ntoa(*(struct in_addr*)&ip));

	f_filter = mt_create_filefilter();
	mt_add_filter(f_filter, 1, ".c");
	mt_listfolder("./", f_filter, &dirs);
	mt_delete_filefilter(f_filter);
	if (dirs)
		mt_freefolder(dirs);
	
	return 0;
}

#endif


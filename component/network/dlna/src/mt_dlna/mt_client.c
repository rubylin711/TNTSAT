#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <fcntl.h>

int main(int argc, char* argv[])
{
	int sk;
	int rlen;
	char	buf[128];

	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(6886);
	addr.sin_addr.s_addr = inet_addr(argv[1]);

	sk = socket(AF_INET, SOCK_STREAM, 0);
	if (!connect(sk, (struct sockaddr*)&addr, sizeof(addr))) {
		rlen = recv(sk, buf, 127, 0);
		if (rlen > 0) {
			buf[rlen] = 0;
			printf("\nRecv data:\t%s\n", buf);
		}

	}

	close(sk);

	return 0;
		
}



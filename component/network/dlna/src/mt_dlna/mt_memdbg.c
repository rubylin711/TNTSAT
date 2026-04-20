#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

int main(void)
{
	char cmd[128];
	char result[4096];
	void *ptr;
	int pid, len;

	pid = getpid();
	sprintf(cmd, "cat /proc/%d/maps",  pid);
	len = __mt_cmdresult(cmd, result, 4095);
	
	result[len] = 0;
	printf("\npid = %d\n%s\n", pid, result);
	ptr = malloc(1 << 21);
	len = __mt_cmdresult(cmd, result, 4095);
	
	result[len] = 0;
	printf("\nAlloc 2M\n%s\n", result);
	free(ptr);
	len = __mt_cmdresult(cmd, result, 4095);
	
	result[len] = 0;
	printf("\nFree 2M\n%s\n", result);


	return 0;
}



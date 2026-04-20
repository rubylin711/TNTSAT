#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>

static volatile int odd_even = 0;

static void sig_SIGALRM(int signum)
{
	if (odd_even == 0) {
		odd_even = 1;
		printf("odd_even = 1\n");
	} else {
		odd_even = 0;
		printf("odd_even = 0\n");
	}
}

int main(int argc, char **argv)
{
	int interval;
	int ret;
	struct itimerval value, old_value;
	unsigned long long i = 0;

	interval = atoi(argv[1]);
	printf("interval = %d us\n", interval);

	value.it_value.tv_sec = 0;
	value.it_value.tv_usec = 1000;
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_usec = interval;

	signal(SIGALRM, sig_SIGALRM);
	ret = setitimer(ITIMER_REAL, &value, &old_value);
	printf("ret = %d\n", ret);

	while (1) {
		if (odd_even == 0) {
			i++;
		} else {
			printf("pause %d us\n", interval);
			pause();
		}
	}

	return 0;
}

/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>

int usleep(useconds_t usec)
{
	struct timespec ts;
	int ret;

	if (usec == 0) {
		return 0;
	}

	ts.tv_sec = (long)(usec / 1000000);
	ts.tv_nsec = (long)(usec % 1000000) * 1000;
	while (((ret = nanosleep(&ts, &ts)) == -1) && errno == EINTR);
	return ret;
}

#define	_PATH_BSHELL	"/bin/sh"
#define _REENTRANT
#define rwlock_t pthread_rwlock_t
#define rwlock_rdlock pthread_rwlock_rdlock
#define rwlock_unlock pthread_rwlock_unlock
#define rwlock_wrlock pthread_rwlock_wrlock
#define RWLOCK_INITIALIZER PTHREAD_RWLOCK_INITIALIZER
#define __readlockenv() 0
#define __unlockenv() 0

static struct pid {
	struct pid *next;
	FILE *fp;
#ifdef _REENTRANT
	int fd;
#endif
	pid_t pid;
} *pidlist;

#ifdef _REENTRANT
static rwlock_t pidlist_lock = RWLOCK_INITIALIZER;
#endif

int system(const char *command)
{
	pid_t pid;
	sig_t intsave, quitsave;
	sigset_t mask, omask;
	int pstat;
	char *argp[] = {"sh", "-c", NULL, NULL};

	if (!command)		/* just checking... */
		return(1);

	argp[2] = (char *)command;

	sigemptyset(&mask);
	sigaddset(&mask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &mask, &omask);
	switch (pid = vfork()) {
	case -1:			/* error */
		sigprocmask(SIG_SETMASK, &omask, NULL);
		return(-1);
	case 0:				/* child */
		sigprocmask(SIG_SETMASK, &omask, NULL);
		execve(_PATH_BSHELL, argp, __environ);
		_exit(127);
	}

	intsave = signal(SIGINT, SIG_IGN);
	quitsave = signal(SIGQUIT, SIG_IGN);
	pid = waitpid(pid, (int *)&pstat, 0);
	sigprocmask(SIG_SETMASK, &omask, NULL);
	(void)signal(SIGINT, intsave);
	(void)signal(SIGQUIT, quitsave);
	return (pid == -1 ? -1 : pstat);
}

FILE *popen(const char *command, const char *type)
{
	struct pid *cur, *old;
	FILE *iop;
	const char * volatile xtype = type;
	int pdes[2], pid, serrno;
	volatile int twoway;
	int flags;

	assert(command != NULL);
	assert(xtype != NULL);

	flags = strchr(xtype, 'e') ? O_CLOEXEC : 0;
	if (strchr(xtype, '+')) {
		int stype = flags ? (SOCK_STREAM | SOCK_CLOEXEC) : SOCK_STREAM;
		twoway = 1;
		xtype = "r+";
		if (socketpair(AF_LOCAL, stype, 0, pdes) < 0)
			return NULL;
	} else  {
		twoway = 0;
		xtype = strrchr(xtype, 'r') ? "r" : "w";
		if (pipe2(pdes, flags) == -1)
			return NULL;
	}

	if ((cur = malloc(sizeof(struct pid))) == NULL) {
		(void)close(pdes[0]);
		(void)close(pdes[1]);
		errno = ENOMEM;
		return (NULL);
	}

	(void)rwlock_rdlock(&pidlist_lock);
	(void)__readlockenv();
	switch (pid = vfork()) {
	case -1:			/* Error. */
		serrno = errno;
		(void)__unlockenv();
		(void)rwlock_unlock(&pidlist_lock);
		free(cur);
		(void)close(pdes[0]);
		(void)close(pdes[1]);
		errno = serrno;
		return (NULL);
		/* NOTREACHED */
	case 0:				/* Child. */
		/* POSIX.2 B.3.2.2 "popen() shall ensure that any streams
		   from previous popen() calls that remain open in the
		   parent process are closed in the new child process. */
		for (old = pidlist; old; old = old->next)
#ifdef _REENTRANT
			close(old->fd); /* don't allow a flush */
#else
			close(fileno(old->fp)); /* don't allow a flush */
#endif

		if (*xtype == 'r') {
			(void)close(pdes[0]);
			if (pdes[1] != STDOUT_FILENO) {
				(void)dup2(pdes[1], STDOUT_FILENO);
				(void)close(pdes[1]);
			}
			if (twoway)
				(void)dup2(STDOUT_FILENO, STDIN_FILENO);
		} else {
			(void)close(pdes[1]);
			if (pdes[0] != STDIN_FILENO) {
				(void)dup2(pdes[0], STDIN_FILENO);
				(void)close(pdes[0]);
			}
		}

		execl(_PATH_BSHELL, "sh", "-c", command, NULL);
		_exit(127);
		/* NOTREACHED */
	}
	(void)__unlockenv();

	/* Parent; assume fdopen can't fail. */
	if (*xtype == 'r') {
		iop = fdopen(pdes[0], xtype);
#ifdef _REENTRANT
		cur->fd = pdes[0];
#endif
		(void)close(pdes[1]);
	} else {
		iop = fdopen(pdes[1], xtype);
#ifdef _REENTRANT
		cur->fd = pdes[1];
#endif
		(void)close(pdes[0]);
	}

	/* Link into list of file descriptors. */
	cur->fp = iop;
	cur->pid =  pid;
	cur->next = pidlist;
	pidlist = cur;
	(void)rwlock_unlock(&pidlist_lock);

	return (iop);
}

/*
 * pclose --
 *	Pclose returns -1 if stream is not associated with a `popened' command,
 *	if already `pclosed', or waitpid returns an error.
 */
int pclose(FILE *iop)
{
	struct pid *cur, *last;
	int pstat;
	pid_t pid;

	assert(iop != NULL);

	rwlock_wrlock(&pidlist_lock);

	/* Find the appropriate file pointer. */
	for (last = NULL, cur = pidlist; cur; last = cur, cur = cur->next)
		if (cur->fp == iop)
			break;
	if (cur == NULL) {
		(void)rwlock_unlock(&pidlist_lock);
		return (-1);
	}

	(void)fclose(iop);

	/* Remove the entry from the linked list. */
	if (last == NULL)
		pidlist = cur->next;
	else
		last->next = cur->next;

	(void)rwlock_unlock(&pidlist_lock);

	do {
		pid = waitpid(cur->pid, &pstat, 0);
	} while (pid == -1 && errno == EINTR);

	free(cur);

	return (pid == -1 ? -1 : pstat);
}

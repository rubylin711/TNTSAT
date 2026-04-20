#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <malloc.h>
#include <stdarg.h>

#define TEST_SIZE (1024*1024*20)

#define   noinline    __attribute__((__noinline__))

volatile int noinline Printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	return vprintf(format, args);
}

volatile void noinline *Memset(void *s, int c, size_t n)
{
	return memset(s, c, n);
//	return NULL;
}

volatile void noinline *Malloc(size_t size)
{
	return malloc(size);
}

volatile void noinline *Calloc(size_t nmemb, size_t size)
{
	return calloc(nmemb, size);
}

volatile void noinline *Realloc(void *ptr, size_t size)
{
	return realloc(ptr, size);
}

volatile void noinline *Memalign(size_t alignment, size_t size)
{
	return memalign(alignment, size);
}

volatile void noinline *Mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
	return mmap(addr, length, prot, flags, fd, offset);
}

volatile void noinline *e(int size)
{
	char *p = NULL;
	p = Mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	Memset(p, 1, size);
	Printf("mmap: %d, p = %p\n", size, p);
	return p;
}

volatile void noinline *d(int size)
{
	return e(size);
}


volatile void noinline *c(int size)
{
	return d(size);
}


volatile void noinline *b(int size)
{
	return c(size);
}

volatile void noinline *a(int size)
{
	return b(size);
}

int main(int argc, char **argv)
{
	char *p = NULL;
	int size = TEST_SIZE;

	while (1) {
#if 0
		p = Malloc(size);
		Memset(p, 1, size);
		Printf("malloc: %d, p = %p\n", size, p);
#elif 0
		p = Calloc(1, size);
		Printf("calloc: %d, p = %p\n", size, p);
#elif 0
		p = Realloc(p, size);
		Memset(p, 1, size);
		Printf("realloc: %d, p = %p\n", size, p);
		size += TEST_SIZE;
#elif 0
		p = Memalign(4096, size);
		Memset(p, 1, size);
		Printf("memalign: %d, p = %p\n", size, p);
#elif 1
		p = Malloc(size);
		Memset(p, 1, size);
		Printf("malloc: %d, p = %p\n", size, p);
	#if 1
		p = Mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
		Memset(p, 1, size);
		Printf("mmap: %d, p = %p\n", size, p);
	#else
		a(size);
	#endif
#else
	#if 1
		p = Mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
		Memset(p, 1, size);
		Printf("mmap: %d, p = %p\n", size, p);
	#else
		a(size);
	#endif
#endif
	}

	return 0;
}

/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <malloc.h>
#include <alloca.h>
#include "sanitize.h"
#include "mt_type.h"
#include "mt_common.h"
#include "mt_sanitize.h"

extern int lmx;
extern int lmy;
extern int lmz;
int c[100];
int d[100];
int e[100];
void fc(int *xx);

void fc(int *xx)
{
	*xx = 1;
}

void *sanitize(void *param)
{
	int a[100];
	int b[100];
	float ft[100];
	long p;
	int i;
	char *q = (void *)b;
	char *r = (void *)d;		//e
	char *s = (void *)&b[100];
	char *t = (void *)&d[100];	//e
	char *alloca_p;
	int *span0, *span1, *span;
	int *span0_untagged, *span1_untagged;
	void *quarantine = NULL;

	printf("line %d, test quarantine ? 1 : test, 0 : not test\n", __LINE__);
	scanf("%ld", &p);
	if (p == 1) {
		while (1) {
			quarantine = realloc(quarantine, 3000);
			printf("quarantine = %p\n", quarantine);
		}
	}

	printf("line %d, test free buffer which malloc in other .c file: %p\n", __LINE__, param);
#ifdef CONFIG_MT_SANITIZE_TAG
	printf("free wrong tag_addr ? 1 : tag_wrong, 0 : tag_right\n");
	scanf("%ld", &p);
	if (p == 1) {
		printf("what tag issue ? 1 : tag_value_wrong, 0 : tag_place_wrong\n");
		scanf("%ld", &p);
		if (p == 1) {
			param = __tag_set(param, __tag_get(param) + 1);
			printf("tag value wrong: %p\n", param);
			free(param);
		} else {
			u8 tag = __tag_get(param);
			param = __untagged_addr(param);
			param = param + ((u64)tag << 48);
			printf("tag place wrong: %p\n", param);
			free(param);
		}
	} else {
		printf("free tag_right: %p\n", param);
		free(param);
	}
#else
	free(param);
#endif

	memset((void *)a, 0, sizeof(a));
	memset((void *)b, 0, sizeof(b));
	memset((void *)c, 0, sizeof(c));
	memset((void *)d, 0, sizeof(d));

	printf("line %d, intput offset for a\n", __LINE__);
	scanf("%ld", &p);
	printf("a = %p, offset = %ld\n", a, p);
	a[p] = 1;

	printf("line %d, intput offset for b\n", __LINE__);
	scanf("%ld", &p);
	printf("b = %p, offset = %ld\n", b, p);
	b[p] = 2;

	printf("line %d, intput offset for c\n", __LINE__);
	scanf("%ld", &p);
	printf("c = %p, offset = %ld\n", c, p);
	c[p] = 3;

	printf("line %d, intput offset for d\n", __LINE__);
	scanf("%ld", &p);
	printf("d = %p, offset = %ld\n", d, p);
	d[p] = 4;

	printf("line %d, intput offset for e\n", __LINE__);
	scanf("%ld", &p);
	printf("e = %p, offset = %ld\n", e, p);
	e[p] = 5;

	printf("line %d, intput offset for float buffer: ft\n", __LINE__);
	scanf("%ld", &p);
	printf("ft = %p, offset = %ld\n", ft, p);
	ft[p] = 5.1;
	ft[p + 1] = 0.2;
	printf("line %d, ft[%ld] = %f, ft[%ld] = %f, ft[%ld] + ft[%ld] = %f\n", __LINE__, p, ft[p], p + 1, ft[p + 1], p, p + 1, ft[p] + ft[p + 1]);

	printf("line %d, test alloca_p ? 1: yes, 0: no\n", __LINE__);
	scanf("%ld", &p);
	if (p == 1) {
		alloca_p = alloca(100);
		printf("alloca_p = %p\n", alloca_p);
		printf("line %d, intput offset for alloca\n", __LINE__);
		scanf("%ld", &p);
		printf("offset = %ld\n", p);
		alloca_p[p] = 3;
	}

	printf("line %d, test stack buffer underflow, input -1\n", __LINE__);
	scanf("%ld", &p);
	printf("q = %p, offset = %ld\n", q, p);
	q[p] = 0;

	printf("line %d, test global buffer underflow in same file, input -1\n", __LINE__);
	scanf("%ld", &p);
	printf("r = %p, offset = %ld\n", r, p);
	r[p] = 0;

	printf("line %d, test global buffer underflow in other file, input -1\n", __LINE__);
	scanf("%ld", &p);
	span0 = &lmy;		//lmz
	printf("span0 = %p, offset = %ld\n", span0, p);
	span0[p] = 0;

	printf("line %d, test stack buffer overflow ? 1 : yes, 0 : no\n", __LINE__);
	scanf("%ld", &p);
	if (p == 1) {
		printf("s = %p, offset = %ld\n", s, (long)0);
		s[0] = 0;
	}

	printf("line %d, test global buffer overflow in same file ? 1 : yes, 0 : no\n", __LINE__);
	scanf("%ld", &p);
	if (p == 1) {
		printf("t = %p, offset = %ld\n", t, (long)0);
		t[0] = 0;
	}

	printf("line %d, test global buffer overflow in other file, input 1\n", __LINE__);
	scanf("%ld", &p);
	span1 = &lmy;		//lmz
	printf("span1 = %p, offset = %ld\n", span1, p);
	span1[p] = 0;

	printf("line %d, test pointer span to next buffer case for stack buffer ? 1: yes, 0: no\n", __LINE__);
	scanf("%ld", &p);
	if (p == 1) {
		if (untagged_addr((void *)a) < untagged_addr((void *)b)) {
			span0 = a + 1;
			span1 = b;
		} else {
			span0 = b + 1;
			span1 = a;
		}
		span0_untagged = untagged_addr(span0);
		span1_untagged = untagged_addr(span1);
		printf("span0 = %p, span1 = %p, span0_untagged = %p, span1_untagged = %p\n", span0, span1, span0_untagged, span1_untagged);

		i = 0;
		while (span0_untagged + i < span1_untagged) {
			i++;
		};
		printf("span0 + %d = %p\n", i, span0 + i);
		*(span0 + i) = 1;
		memset(span0 + i, 0, 4);
		span = span0 + i;
		*span = 1;

		p = (long)span;
		*(int *)p = 1;

		fc(span0 + i);
		fc(span);

		i = 0;
		while ((span1_untagged - i) > (span0_untagged + 10)) {
			i++;
		};
		*(span1 - i) = 1;
		memset(span1 - i, 0, 4);
		span = span1 - i;
		*span = 1;
	}

	printf("line %d, test pointer span to next buffer case for global buffer ? 1: yes, 0: no\n", __LINE__);
	scanf("%ld", &p);
	if (p == 1) {
		if (untagged_addr((void *)c) < untagged_addr((void *)d)) {
			span0 = c + 1;
			span1 = d;
		} else {
			span0 = d + 1;
			span1 = c;
		}
		span0_untagged = untagged_addr(span0);
		span1_untagged = untagged_addr(span1);
		printf("span0 = %p, span1 = %p, span0_untagged = %p, span1_untagged = %p\n", span0, span1, span0_untagged, span1_untagged);

		i = 0;
		while (span0_untagged + i < span1_untagged) {
			i++;
		};
		printf("span0 + %d = %p\n", i, span0 + i);
		*(span0 + i) = 1;
		memset(span0 + i, 0, 4);
		span = span0 + i;
		*span = 1;

		p = (long)span;
		*(int *)p = 1;

		fc(span0 + i);
		fc(span);

		i = 0;
		while ((span1_untagged - i) > (span0_untagged + 10)) {
			i++;
		};
		*(span1 - i) = 1;
		memset(span1 - i, 0, 4);
		span = span1 - i;
		*span = 1;
	}

	printf("line %d, test pointer span to next buffer case for heap buffer with malloc ? 1: yes, 0: no\n", __LINE__);
	scanf("%ld", &p);
	if (p == 1) {
		s = malloc(100);
		t = malloc(100);

		if (untagged_addr((void *)s) < untagged_addr((void *)t)) {
			span0 = (void *)(s + 1);
			span1 = (void *)t;
		} else {
			span0 = (void *)(t + 1);
			span1 = (void *)s;
		}
		span0_untagged = untagged_addr(span0);
		span1_untagged = untagged_addr(span1);
		printf("span0 = %p, span1 = %p, span0_untagged = %p, span1_untagged = %p\n", span0, span1, span0_untagged, span1_untagged);

		i = 0;
		while (span0_untagged + i < span1_untagged) {
			i++;
		};
		printf("span0 + %d = %p\n", i, span0 + i);
		*(span0 + i) = 1;
		memset(span0 + i, 0, 4);
		span = span0 + i;
		*span = 1;

		p = (long)span;
		*(int *)p = 1;

		fc(span0 + i);
		fc(span);

		i = 0;
		while ((span1_untagged - i) > (span0_untagged + 10)) {
			i++;
		};
		*(span1 - i) = 1;
		memset(span1 - i, 0, 4);
		span = span1 - i;
		*span = 1;

		free(s);
		free(t);
	}

	printf("line %d, test pointer span to next buffer case for heap buffer with mmap ? 1: yes, 0: no\n", __LINE__);
	scanf("%ld", &p);
	if (p == 1) {
		s = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
		t = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

		if (untagged_addr((void *)s) < untagged_addr((void *)t)) {
			span0 = (void *)(s + 1);
			span1 = (void *)t;
		} else {
			span0 = (void *)(t + 1);
			span1 = (void *)s;
		}
		span0_untagged = untagged_addr(span0);
		span1_untagged = untagged_addr(span1);
		printf("span0 = %p, span1 = %p, span0_untagged = %p, span1_untagged = %p\n", span0, span1, span0_untagged, span1_untagged);

		i = 0;
		while (span0_untagged + i < span1_untagged) {
			i++;
		};
		printf("span0 + %d = %p\n", i, span0 + i);
		*(span0 + i) = 1;
		memset(span0 + i, 0, 4);
		span = span0 + i;
		*span = 1;

		p = (long)span;
		*(int *)p = 1;

		fc(span0 + i);
		fc(span);

		i = 0;
		while ((span1_untagged - i) > (span0_untagged + 10)) {
			i++;
		};
		*(span1 - i) = 1;
		memset(span1 - i, 0, 4);
		span = span1 - i;
		*span = 1;

		munmap(s, 4096);
		munmap(t, 4096);
	}

	printf("line %d, test pointer span to next buffer case for heap buffer with mmz ? 1: yes, 0: no\n", __LINE__);
	scanf("%ld", &p);
	if (p == 1) {
		phys_addr_t s_phy, t_phy;

		s_phy = mt_mmz_new(4096, 0, NULL, "hwasan_span_mmz");
		s = mt_mmz_map(s_phy, 1);
		t_phy = mt_mmz_new(4096, 0, NULL, "hwasan_span_mmz");
		t = mt_mmz_map(t_phy, 1);

		printf("test self access\n");
		*s = 1;
		*t = 2;

		if (untagged_addr((void *)s) < untagged_addr((void *)t)) {
			span0 = (void *)(s + 1);
			span1 = (void *)t;
		} else {
			span0 = (void *)(t + 1);
			span1 = (void *)s;
		}
		span0_untagged = untagged_addr(span0);
		span1_untagged = untagged_addr(span1);
		printf("span0 = %p, span1 = %p, span0_untagged = %p, span1_untagged = %p\n", span0, span1, span0_untagged, span1_untagged);

		i = 0;
		while (span0_untagged + i < span1_untagged) {
			i++;
		};
		printf("span0 + %d = %p\n", i, span0 + i);
		*(span0 + i) = 1;
		memset(span0 + i, 0, 4);
		span = span0 + i;
		*span = 1;

		p = (long)span;
		*(int *)p = 1;

		fc(span0 + i);
		fc(span);

		i = 0;
		while ((span1_untagged - i) > (span0_untagged + 10)) {
			i++;
		};
		*(span1 - i) = 1;
		memset(span1 - i, 0, 4);
		span = span1 - i;
		*span = 1;

		mt_mmz_unmap(s);
		mt_mmz_unmap(t);
		mt_mmz_delete(s_phy);
		mt_mmz_delete(t_phy);
	}

	q = malloc(100);

	printf("line %d, test heap buffer underflow, input -1\n", __LINE__);
	scanf("%ld", &p);
	printf("q = %p, offset = %ld\n", q, p);
	q[p] = 0;

	printf("line %d, test heap buffer overflow, input 100\n", __LINE__);
	scanf("%ld", &p);
	printf("q = %p, offset = %ld\n", q, p);
	q[p] = 0;

	printf("line %d, test memory leak ? 1: yes, 0: no\n", __LINE__);
	scanf("%ld", &p);
	if (p == 1) {
		q = malloc(100);
	}

	printf("line %d, intput offset for q\n", __LINE__);
	scanf("%ld", &p);
	printf("q = %p, offset = %ld\n", q, p);
	q[p] = 5;

	printf("line %d, test free wrong place ? 1: yes, 0: no\n", __LINE__);
	scanf("%ld", &p);
	if (p == 1) {
		free(q + 1);
	} else {
		free(q);
	}

	printf("line %d, test double free ? 1: yes, 0: no\n", __LINE__);
	scanf("%ld", &p);
	if (p == 1) {
		free(q);
	}

	printf("line %d, test use after free with not malloc again ? 1: yes, 0: no\n", __LINE__);
	scanf("%ld", &p);
	if (p == 1) {
		printf("q = %p, offset = %ld\n", q, (long)0);
		q[0] = 0;
	}

	printf("line %d, test use after free with malloc again and got the same area ? 1: yes, 0: no\n", __LINE__);
	scanf("%ld", &p);
	if (p == 1) {
		s = malloc(0x1000000);
		printf("s = %p, offset = %ld\n", s, (long)0);
		s[0] = 1;
		free(s);
		for (i = 0; ; i++) {
			t = malloc(0x1000000);
			t[0] = 1;
			if (untagged_addr(t) != untagged_addr(s)) {
				free(t);
			} else {
				break;
			}
		}
		printf("line %d, 0x1000000B, i = %d, s = %p, t = %p, untagged_addr(s) = %p, untagged_addr(t) = %p\n", __LINE__, i, (void *)s, (void *)t, untagged_addr(s), untagged_addr(t));
		s[0] = 1;
		free(t);
		s[0] = 1;
	}

	printf("line %d, test redzone size\n", __LINE__);
	s = malloc(0x1000000);
	t = malloc(0x1000000);
	free(s);
	free(t);
	printf("line %d, malloc 0x1000000, gap = %ld\n", __LINE__, untagged_addr(s) > untagged_addr(t) ? untagged_addr(s) - untagged_addr(t) : untagged_addr(t) - untagged_addr(s));
	s = malloc(4096);
	t = malloc(4096);
	free(s);
	free(t);
	printf("line %d, malloc 4096, gap = %ld\n", __LINE__, untagged_addr(s) > untagged_addr(t) ? untagged_addr(s) - untagged_addr(t) : untagged_addr(t) - untagged_addr(s));
	s = malloc(1000);
	t = malloc(1000);
	free(s);
	free(t);
	printf("line %d, malloc 1000, gap = %ld\n", __LINE__, untagged_addr(s) > untagged_addr(t) ? untagged_addr(s) - untagged_addr(t) : untagged_addr(t) - untagged_addr(s));
	s = malloc(100);
	t = malloc(100);
	free(s);
	free(t);
	printf("line %d, malloc 100, gap = %ld\n", __LINE__, untagged_addr(s) > untagged_addr(t) ? untagged_addr(s) - untagged_addr(t) : untagged_addr(t) - untagged_addr(s));
	s = malloc(10);
	t = malloc(10);
	free(s);
	free(t);
	printf("line %d, malloc 10, gap = %ld\n", __LINE__, untagged_addr(s) > untagged_addr(t) ? untagged_addr(s) - untagged_addr(t) : untagged_addr(t) - untagged_addr(s));

	return NULL;
}


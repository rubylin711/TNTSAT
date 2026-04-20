#ifndef __LYNX_INIT_H__
#define __LYNX_INIT_H__

#if defined(CONFIG_LYNX_OS_LINUX)
#include <linux/interrupt.h>
#else
#include <os/mtos_sem.h>

#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->p_next, type, member)

/* FIXME: the INIT_LIST_HEAD in the inc/util/list.h is different to the linux's */
#ifdef INIT_LIST_HEAD
#undef INIT_LIST_HEAD
#endif
static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->p_next = list;
	list->p_prev = list;
}

#define list_next_entry(pos, member) \
	list_entry((pos)->member.p_next, typeof(*(pos)), member)

#ifdef list_for_each_entry
#undef list_for_each_entry
#endif
#define list_for_each_entry(pos, head, member)				\
	for (pos = list_first_entry(head, typeof(*pos), member);	\
	     &pos->member != (head);					\
	     pos = list_next_entry(pos, member))

#ifndef test_bit
#define test_bit(bit, var)	      ((*var) & (1 << (bit)))
#endif

struct timer_list {
	struct list_head entry;
	unsigned long expires;
	void (*function)(unsigned long);
	unsigned long data;

	unsigned long priv;
};

struct completion
{
	unsigned int done;
	os_sem_t    wait;
};
#endif // !defined(CONFIG_LYNX_OS_LINUX)

#if defined(CONFIG_LYNX_OS_LINUX)
typedef spinlock_t OS_LOCK_TYPE;
#elif defined(CONFIG_LYNX_OS_UCOS)
/* The uCoS doesn't has spin lock, and can't use mutex in the ISR. 
	So, we use mtos_critical_enter() to do protect by enabling the CONFIG_LOCK_BY_ISR */
#define CONFIG_LOCK_BY_ISR	1
#ifdef CONFIG_LOCK_BY_ISR
typedef unsigned long OS_LOCK_TYPE;
#else	// CONFIG_LOCK_BY_ISR
typedef os_sem_t OS_LOCK_TYPE;
#endif	// CONFIG_LOCK_BY_ISR
#endif	// defined(CONFIG_LYNX_OS_UCOS)

#if defined(CONFIG_LYNX_OS_LINUX)
typedef struct completion OS_COMPLETION;
#elif defined(CONFIG_LYNX_OS_UCOS)
typedef os_sem_t OS_COMPLETION;
#endif

struct linux_priv {
	struct timer_list wm_timer;
};

struct lynx *os_dep_create(void);
int os_dep_init(struct lynx *lnx);
void os_dep_deinit(struct lynx *lnx);
int os_wait_init_complete(struct lynx *lnx);
int os_wait_link_st_complete(struct lynx *lnx);
int os_wakeup_wait_queue(void *wait_queue);
int os_init_completion(OS_COMPLETION *comp);
int os_wait_for_completion_interruptible(OS_COMPLETION *x, int timeout);
int os_complete(OS_COMPLETION *x);
void os_completion_done(OS_COMPLETION *x);

int os_dep_init_wm_manager(struct lynx *lnx);
void os_dep_deinit_wm_manager(void);
int os_check_thread_stop(void);
void os_thread_sleep(unsigned int msec);
void os_thread_exit(void);

unsigned int os_current_time(void);
void os_init_timer(struct timer_list *timer, void *func, unsigned long data);
void os_deinit_timer(struct timer_list *timer);
int os_add_timer(struct timer_list *timer, unsigned int interval);
void os_del_timer(struct timer_list *wm_timer);

char *os_api_alloc(int size, int flags);
void os_api_free(void *);
int os_api_lock_init(OS_LOCK_TYPE *lock);
int os_api_lock_deinit(OS_LOCK_TYPE *lock);
int os_api_lock(OS_LOCK_TYPE *lock);
int os_api_unlock(OS_LOCK_TYPE *lock);
int os_api_dsr_lock(OS_LOCK_TYPE *lock);
int os_api_dsr_unlock(OS_LOCK_TYPE *lock);
int os_api_isr_lock(OS_LOCK_TYPE *lock, unsigned long *flags);
int os_api_isr_unlock(OS_LOCK_TYPE *lock, unsigned long *flags);
int os_api_rand(unsigned int seed);

#endif	// __LYNX_INIT_H__

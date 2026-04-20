#define MT_DECLARE_MUTEX(x) DEFINE_SEMAPHORE(x, 1)
#define MT_INIT_MUTEX(x)  sema_init(x, 1)

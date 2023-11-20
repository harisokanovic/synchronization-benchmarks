// XXX

//uint64_t test_lock_xxx;
//uint64_t test_lock;

// Based on arch/arm/include/asm/atomic.h from Linux
// Copied from Lockhammer lk_atomic.h
static inline void cpu_relax (void) {
#if defined(__x86_64__)
	asm volatile ("pause" : : : "memory" );
#elif defined (__aarch64__) && defined(RELAX_IS_ISB)
	asm volatile ("isb" : : : "memory" );
#elif defined (__aarch64__)
	asm volatile ("yield" : : : "memory" );
#else
#error Unsupported
#endif
}

static inline unsigned long lock_acquire (uint64_t *lock, unsigned long threadnum) {
  uint32_t* const pnext = (uint32_t*)lock;
  uint32_t* const powner = &pnext[1];

  const uint32_t my_ticket = __atomic_fetch_add(pnext, (uint32_t)1, __ATOMIC_RELAXED);

  uint32_t now_serving = my_ticket;
  do {
    uint32_t relax_count = (my_ticket - now_serving) * 1000;
    while (relax_count--) {
      cpu_relax();
    }

    now_serving = __atomic_load_n(powner, __ATOMIC_ACQUIRE);
  } while(now_serving != my_ticket);

  return 0;
}

static inline void lock_release (uint64_t *lock, unsigned long threadnum) {
  uint32_t* const pnext = (uint32_t*)lock;
  uint32_t* const powner = &pnext[1];

  __atomic_fetch_add(powner, (uint32_t)1, __ATOMIC_RELEASE);
}

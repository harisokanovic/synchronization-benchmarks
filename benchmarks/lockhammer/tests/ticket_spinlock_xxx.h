// XXX

uint32_t xxx_relax_count;

#define RELAX_IS_ISB 1

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
  const uint32_t initial_lock_depth = my_ticket - __atomic_load_n(powner, __ATOMIC_ACQUIRE);

  for(;;) {
    const uint32_t now_serving = __atomic_load_n(powner, __ATOMIC_ACQUIRE);
    if (now_serving == my_ticket) {
      break;
    }

    const uint32_t current_lock_depth = my_ticket - now_serving;
    uint32_t relax_count = current_lock_depth * xxx_relax_count;
    while (relax_count--) {
      cpu_relax();
    }
  }

  return initial_lock_depth;
}

static inline void lock_release (uint64_t *lock, unsigned long threadnum) {
  uint32_t* const pnext = (uint32_t*)lock;
  uint32_t* const powner = &pnext[1];

  __atomic_fetch_add(powner, (uint32_t)1, __ATOMIC_RELEASE);
}

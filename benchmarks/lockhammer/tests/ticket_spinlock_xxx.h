// XXX

static inline unsigned long lock_acquire (uint64_t *lock, unsigned long threadnum) {
  uint16_t* const pnext = (uint16_t*)lock;
  uint16_t* const powner = &pnext[1];

  const uint16_t my_ticket = __atomic_fetch_add(pnext, (uint16_t)1, __ATOMIC_SEQ_CST);

  uint16_t now_serving;
  do {
    now_serving = __atomic_load_n(powner, __ATOMIC_SEQ_CST);
  } while(now_serving != my_ticket);

  return 0;
}

static inline void lock_release (uint64_t *lock, unsigned long threadnum) {
  uint16_t* const pnext = (uint16_t*)lock;
  uint16_t* const powner = &pnext[1];

  __atomic_fetch_add(powner, (uint16_t)1, __ATOMIC_SEQ_CST);
}

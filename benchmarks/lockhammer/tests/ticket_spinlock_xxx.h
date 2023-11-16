// XXX

static inline unsigned long lock_acquire (uint64_t *lock, unsigned long threadnum) {
  uint32_t* const pnext = (uint32_t*)lock;
  uint32_t* const powner = &pnext[1];

  const uint32_t my_ticket = __atomic_fetch_add(pnext, (uint32_t)1, __ATOMIC_SEQ_CST);

  uint32_t now_serving;
  do {
    now_serving = __atomic_load_n(powner, __ATOMIC_SEQ_CST);
  } while(now_serving != my_ticket);

  return 0;
}

static inline void lock_release (uint64_t *lock, unsigned long threadnum) {
  uint32_t* const pnext = (uint32_t*)lock;
  uint32_t* const powner = &pnext[1];

  __atomic_fetch_add(powner, (uint32_t)1, __ATOMIC_SEQ_CST);
}



/* with uint16_t + __ATOMIC_SEQ_CST
       │     lock_acquire():                                                                                                                                                                                                                                                     ▒
  0.00 │       ldaddalh w21, w2, [x19]                                                                                                                                                                                                                                           ▒
  0.00 │       and      w2, w2, #0xffff                                                                                                                                                                                                                                          ▒
       │                                                                                                                                                                                                                                                                         ▒
       │     uint16_t now_serving;                                                                                                                                                                                                                                               ▒
       │     do {                                                                                                                                                                                                                                                                ▒
       │     now_serving = __atomic_load_n(powner, __ATOMIC_SEQ_CST);                                                                                                                                                                                                            ▒
 66.25 │1a4:   ldarh    w1, [x27]                                                                                                                                                                                                                                                ▒
       │     } while(now_serving != my_ticket);                                                                                                                                                                                                                                  ▒
  0.08 │       cmp      w2, w1, uxth                                                                                                                                                                                                                                             ▒
 33.37 │     ↑ b.ne     1a4                                                                                   
 */

/* with uint16_t+ __ATOMIC_RELAXED
       │     lock_acquire():                                                                                                                                                                                                                                                     ▒
  0.00 │       ldaddh w21, w2, [x19]                                                                                                                                                                                                                                             ▒
  0.00 │       and    w2, w2, #0xffff                                                                                                                                                                                                                                            ▒
       │                                                                                                                                                                                                                                                                         ▒
       │     uint16_t now_serving;                                                                                                                                                                                                                                               ▒
       │     do {                                                                                                                                                                                                                                                                ▒
       │     now_serving = __atomic_load_n(powner, __ATOMIC_RELAXED);                                                                                                                                                                                                            ▒
 66.79 │1a4:   ldrh   w1, [x27]                                                                                                                                                                                                                                                  ▒
       │     } while(now_serving != my_ticket);                                                                                                                                                                                                                                  ▒
  0.08 │       cmp    w2, w1, uxth                                                                                                                                                                                                                                               ◆
 32.77 │     ↑ b.ne   1a4
*/



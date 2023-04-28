#pragma once

#include <stdbool.h>
#include <stdatomic.h>
#include <unistd.h>
#include <assert.h>

#define NACCOUNTS ((256))
extern long accounts[NACCOUNTS];
extern atomic_long total_txn;

extern void bank_init(void);

extern void * bank_thread(void * ptr);

extern long read_total_balance(void);

static inline void do_transaction(int from, int to, int dollars)
{
  assert(from < NACCOUNTS);
  assert(to < NACCOUNTS);
  assert(from != to);
  usleep(2); // 100 pages of paperwork!
  accounts[from] -= dollars;
  accounts[to] += dollars;
}

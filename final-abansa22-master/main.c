#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdatomic.h>
#include "bank.h"

static void * audit_worker(void * ptr)
{
  long sum = read_total_balance();
  do {
    usleep(200);
    if (sum != read_total_balance()) {
      printf("Warning: total balance changed\n");
      fflush(stdout);
      pthread_exit(NULL);
    }
  } while (true);
  return NULL; // unreachable
}

int main(int argc, char ** argv)
{
  bank_init();
  if (argc < 2) {
    printf("usage: <nthreads>\n");
    exit(0);
  }

  const int n = atoi(argv[1]);
  if (n < 1 || n > 8) {
    fprintf(stderr, "bad number-of-threads\n");
    exit(0);
  }

  // add some randomness
  srandom(getpid() * getppid());
  // initialize account balance values
  long ref[NACCOUNTS] = {};
  for (int i = 0; i < NACCOUNTS; i++) {
    accounts[i] = random() % 10000;
    ref[i] = accounts[i];
  }

  // create worker threads (the bankers)
  pthread_t pts[8];
  int pipefds[8][2];
  for (int i = 0; i < n; i++) {
    if (pipe(pipefds[i]) != 0) {
      printf("pipe() failed\n");
      exit(0);
    }
    pthread_create(&pts[i], NULL, bank_thread, (void *)pipefds[i]);
  }
  // and the audit worker
  pthread_t pta;
  pthread_create(&pta, NULL, audit_worker, NULL);

  int cmd[3];
  long ntxn = 0; // number of transactions that can be correctly executed
  for (int x = 0; x < 30000; x++) {
    // for each worker
    for (int i = 0; i < n; i++) {
      int from = random() % NACCOUNTS;
      int to = random() % NACCOUNTS;
      int dollars = 1 + (random() & 0xf); // some value between 1 to 16
      ref[from] -= dollars;
      ref[to] += dollars;
      if (from != to) // self-transfer will not be authorized
        ntxn++;
      cmd[0] = from;
      cmd[1] = to;
      cmd[2] = dollars;
      // send the command to the worker i
      write(pipefds[i][1], cmd, sizeof(int) * 3);
    }
  }
  // terminate connection and wait for them to finish
  for (int i = 0; i < n; i++) {
    close(pipefds[i][1]);
    pthread_join(pts[i], NULL);
  }
  // terminate the auditing worker
  pthread_cancel(pta);
  pthread_join(pta, NULL);

  // check for account balances
  for (int i = 0; i < NACCOUNTS; i++) {
    printf("account[%d]: balance: %ld expected: %ld %s\n", i, accounts[i], ref[i], ref[i] == accounts[i] ? "Correct":"Wrong");
  }
  // check for # of transactions really executed
  printf("transactions executed: %ld expected: %ld %s\n", total_txn, ntxn, total_txn == ntxn ? "Correct":"Wrong");
  return 0;
}

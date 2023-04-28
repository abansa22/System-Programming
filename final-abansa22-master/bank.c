#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>
#include <stdatomic.h>
#include "bank.h"


// NACCOUNTS: number of bank accounts
long accounts[NACCOUNTS];

// total number of transactions REALLY executed
// this is equal to the number of times do_transaction() gets called
atomic_long total_txn = 0;

// fine-grained locks for each account
pthread_mutex_t locks[NACCOUNTS];

void bank_init(void)
{
  // TODO: fix this code: print your netid here
  printf("my netid is abansa22\n");
  fflush(stdout);

  for (int i = 0; i < NACCOUNTS; i++) {
    pthread_mutex_init(&locks[i], NULL);
  }
}

void * bank_thread(void * ptr)
{
  int * pipefds = (typeof(pipefds))ptr;
  int cmd[3]; // [0]: "from", [1]: "to", [2]: "dollars"

  // The "banker" thread
  // You should first implement this function to start earning points
  //
  // Read commands from the pipe and execute them in a while-loop.
  // *** You don't need to use locks for passing "./main 1"
  // *** first implement it without locks and if you pass "./main 1", then add locks
  //
  // In each iteration:
  // 1. Use read() to read a command from pipe into the "cmd" buffer
  // 2. acquire locks.
  // 3. call do_transaction() to execute the command. (see bank.h)
  // 4. release locks.
  //
  // Make sure to break the loop gracefully (when no more data is witten to pipe)
  // Code in main.c writes the commands from the other end of the pipe.
  //
  // Be careful with what can be passed to do_transaction.
  // You need to filter out invalid transactions--don't send them to do_transaction().
  // see the assertions in do_transaction(). Those statements must be true.
  // TODO: Your code here
  while(read(pipefds[0], cmd, sizeof(int) * 3) > 0) {
	printf("from: %d, to: %d, amount: %d\n", cmd[0], cmd[1], cmd[2]);
	if (cmd[0] == cmd[1]) continue;
	if (cmd[0] > NACCOUNTS) continue;
	if (cmd[1] > NACCOUNTS) continue;
	if (cmd[2] > 16 || cmd[2] < 0) continue;
	
int acc1, acc2;
	if(cmd[0] < cmd[1]){
		acc1 = cmd[0];
		acc2 = cmd[1];
	} else {
		acc1 = cmd[1];
		acc2 = cmd[0];
	}
	pthread_mutex_lock(&locks[acc1]);
	pthread_mutex_lock(&locks[acc2]);
	do_transaction(cmd[0], cmd[1], cmd[2]);
	total_txn++;
	pthread_mutex_unlock(&locks[acc2]);
	pthread_mutex_unlock(&locks[acc1]);
  }
  return NULL; // just exit the thread
}

long read_total_balance(void)
{
  // Read total balance value
  // This is the last step of your solution.
  // The total balance must be a consistent value everytime you call this function
  // To calculate the correct value, you also need to use locks to protect this "read-only" transaction
  // This is quite similar to the "observer" problem in lab 10
  // The "unsafe" code has been provided below. Fix the code with locks
  // TODO: your code here




  long x = 0;
  for(int i = 0; i < NACCOUNTS; i++) {
	  pthread_mutex_lock(&locks[i]);
  }
  for (int i = 0; i < NACCOUNTS; i++) {
    x += accounts[i];
  }
  for(int i =0; i < NACCOUNTS; i++) {
	  pthread_mutex_unlock(&locks[i]);
  }

  // TODO: your code here




  return x;
}

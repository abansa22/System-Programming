// a toy Airline Reservations System

#include "ars.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>

struct flight_info {
  int next_tid; // +1 everytime
  int nr_booked; // booked <= seats
  struct ticket tickets[]; // all issued tickets of this flight
};

pthread_mutex_t * big_lock;
pthread_cond_t * conds;

int __nr_flights = 0;
int __nr_seats = 0;
struct flight_info ** flights = NULL;

static int ticket_cmp(const void * p1, const void * p2)
{
  const uint64_t v1 = *(const uint64_t *)p1;
  const uint64_t v2 = *(const uint64_t *)p2;
  if (v1 < v2)
    return -1;
  else if (v1 > v2)
    return 1;
  else
    return 0;
}

void tickets_sort(struct ticket * ts, int n)
{
  qsort(ts, n, sizeof(*ts), ticket_cmp);
}

void ars_init(int nr_flights, int nr_seats_per_flight)
{
  flights = malloc(sizeof(*flights) * nr_flights);
  big_lock = (pthread_mutex_t *)malloc(nr_flights * sizeof(pthread_mutex_t));
  conds = (pthread_cond_t *)malloc(nr_flights * sizeof(pthread_cond_t));
  for (int i = 0; i < nr_flights; i++) {
    flights[i] = calloc(1, sizeof(flights[i][0]) + (sizeof(struct ticket) * nr_seats_per_flight));
    flights[i]->next_tid = 1;
    pthread_mutex_init(&big_lock[i], NULL);
    pthread_cond_init(&conds[i], NULL);
  }

  __nr_flights = nr_flights;
  __nr_seats = nr_seats_per_flight;

  // pthread_mutex_init(&big_lock, NULL);
}

int book_flight(short user_id, short flight_number)
{
  // wrong number
  if (flight_number >= __nr_flights || flight_number < 0)
    return -1;

  struct flight_info * fi = flights[flight_number];
  int tid = -1;
  int lock_no = flight_number;
  pthread_mutex_lock(big_lock + lock_no);
  // if flight not full
  if (fi->nr_booked < __nr_seats) {
    tid = fi->next_tid++;
    fi->tickets[fi->nr_booked].uid = user_id;
    fi->tickets[fi->nr_booked].fid = flight_number;
    fi->tickets[fi->nr_booked].tid = tid;
    fi->nr_booked++;
  } else {
    tid = -1;
  }
  pthread_mutex_unlock(big_lock + lock_no);
  return tid;
}

// a helper function for cancel/change
// search a ticket of a flight and return its offset if found
static int search_ticket(struct flight_info * fi, short user_id, int ticket_number)
{
  for (int i = 0; i < fi->nr_booked; i++)
    if (fi->tickets[i].uid == user_id && fi->tickets[i].tid == ticket_number)
      return i; // cancelled

  return -1;
}

bool cancel_flight(short user_id, short flight_number, int ticket_number)
{
  if (flight_number >= __nr_flights || flight_number < 0)
    return false;

  struct flight_info * fi = flights[flight_number];
  int lock_no = flight_number;
  pthread_mutex_lock(big_lock + lock_no);
  int offset = search_ticket(fi, user_id, ticket_number);
  bool cancelled;
  if (offset >= 0) {
    fi->tickets[offset] = fi->tickets[fi->nr_booked-1];
    fi->nr_booked--;
    pthread_cond_signal(&conds[lock_no]);
    cancelled = true; // cancelled
  } else {
    cancelled = false; 
  }
  pthread_mutex_unlock(big_lock + lock_no);
  return cancelled;
}

int change_flight(short user_id, short old_flight_number, int old_ticket_number,
                  short new_flight_number)
{
  // wrong number or no-op
  if (old_flight_number >= __nr_flights ||
      new_flight_number >= __nr_flights ||
      old_flight_number == new_flight_number ||
      new_flight_number < 0 ||
      old_flight_number < 0)
    return -1;

  // two things must be done atomically: (1) cancel the old ticket and (2) book a new ticket
  // if any of the two operations cannot be done, nothing should happen
  // for example, if the new flight has no seat, the existing ticket must remain valid
  // if the old ticket number is invalid, don't acquire a new ticket
  // TODO: your code here
  struct flight_info * oldfi = flights[old_flight_number];
  struct flight_info * newfi = flights[new_flight_number];
  
  int lock_1 = old_flight_number;
  int lock_2 = new_flight_number;
  if (old_flight_number > new_flight_number) {
    lock_1 = new_flight_number;
    lock_2 = old_flight_number;
  }
  pthread_mutex_lock(big_lock + lock_1);
  pthread_mutex_lock(big_lock + lock_2);
  // Search for old ticket
  int offset = search_ticket(oldfi, user_id, old_ticket_number);
  if (offset < 0) {
    pthread_mutex_unlock(big_lock + lock_1);
    pthread_mutex_unlock(big_lock + lock_2);
    return -1;
  }
    // Flight full
  if (newfi->nr_booked >= __nr_seats) {
    pthread_mutex_unlock(big_lock + lock_1);
    pthread_mutex_unlock(big_lock + lock_2);
    return -1;
  }
  
  int book_result = newfi->next_tid++;
  // book now
  newfi->tickets[newfi->nr_booked].uid = user_id;
  newfi->tickets[newfi->nr_booked].fid = new_flight_number;
  newfi->tickets[newfi->nr_booked].tid = book_result;
  newfi->nr_booked++;

  // Finalize cancel
  oldfi->tickets[offset] = oldfi->tickets[oldfi->nr_booked-1];
  oldfi->nr_booked--;
  pthread_cond_signal(&conds[old_flight_number]);
  pthread_mutex_unlock(big_lock + lock_1);
  pthread_mutex_unlock(big_lock + lock_2);
  // Return the booking number
  return book_result;
}

// malloc and dump all tickets in the returned array
struct ticket * dump_tickets(int * n_out)
{
  int n = 0;
  for (int i = 0; i < __nr_flights; i++)
    n += flights[i]->nr_booked;

  struct ticket * const buf = malloc(sizeof(*buf) * n);
  assert(buf);
  n = 0;
  for (int i = 0; i < __nr_flights; i++) {
    memcpy(buf+n, flights[i]->tickets, sizeof(*buf) * flights[i]->nr_booked);
    n += flights[i]->nr_booked;
  }
  *n_out = n; // number of tickets
  return buf;
}

int book_flight_can_wait(short user_id, short flight_number)
{
  // wrong number
  if (flight_number >= __nr_flights || flight_number < 0)
    return -1;

  struct flight_info * fi = flights[flight_number];
  int tid = -1;
  pthread_mutex_lock(&big_lock[flight_number]);
  // if flight not full
  while (fi->nr_booked >= __nr_seats) {
    pthread_cond_wait(&conds[flight_number], &big_lock[flight_number]);
  }
  tid = fi->next_tid++;
  fi->tickets[fi->nr_booked].uid = user_id;
  fi->tickets[fi->nr_booked].fid = flight_number;
  fi->tickets[fi->nr_booked].tid = tid;
  fi->nr_booked++;
  pthread_mutex_unlock(&big_lock[flight_number]);
  return tid;
}

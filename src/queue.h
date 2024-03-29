#ifndef _queue_h_INCLUDED
#define _queue_h_INCLUDED

#define DISCONNECT UINT_MAX
#define DISCONNECTED(IDX) ((int)(IDX) < 0)

struct kissat;

typedef struct links links;
typedef struct queuek queuek;

struct links
{
  unsigned prev, next;
  unsigned stamp;
};

struct queuek
{
  unsigned first, last, stamp;
  struct
  {
    unsigned idx, stamp;
  } search;
};

void kissat_init_queue(queuek *);
void kissat_enqueue(struct kissat *, unsigned idx);
void kissat_dequeue(struct kissat *, unsigned idx);
void kissat_move_to_front(struct kissat *, unsigned idx);

#define LINK(IDX) \
  (solver->links[assert((IDX) < VARS), (IDX)])

#endif

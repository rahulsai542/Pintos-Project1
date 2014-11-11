#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <list.h>
#include <stdint.h>



/* NP Implementation */
/* Floating Point Arithmetic   17.14 fixed point representation*/
/* 17+14 = 31 one bit for signed numbers */
/* x and Y be fixed point; n be integer*/
#define P 17
#define Q 14
#define F 1<<(Q)    //2^q
//Convert n to fixed point: 	n * f
#define TO_FP(n) (n)*(F)
//Convert x to integer (rounding to nearest): 	(x + f / 2) / f if x >= 0,
//(x - f / 2) / f if x <= 0.
#define TO_INT_NEAR(x) ((x) >= 0 ? ((x)+(F)/2) / (F) : ((x)-(F)/2) / (F))
//((x)>=0 ?((x) + (F)/2)/(F) :((x) - (F)/2)/(F))
//((x) >= 0 ? ((x) + (F) / 2) / (F) : ((x) - (F) / 2) / (F))
//Convert x to integer (rounding toward zero): 	x / f
#define TO_INT_ZERO(x) (x)/(F)
//Add x and y: 	x + y
#define ADD_FP(x,y) (x)+(y)
//Subtract y from x: 	x - y
#define SUB_FP(x,y) (x)-(y)
//Add x and n: 	x + n * f
#define ADD_FP_INT(x,n) (x)+(n)*(F)
//Subtract n from x: 	x - n * f
#define SUB_FP_INT(x,n) (x)-(n)*(F)
//Multiply x by y: 	((int64_t) x) * y / f
#define MUL_FP(x,y) ((int64_t)(x)) * (y)/(F)
//Multiply x by n: 	x * n
#define MUL_FP_INT(x,n) (x)*(n)
//Divide x by y: 	((int64_t) x) * f / y
#define DIV_FP(x,y) ((int64_t)(x))*(F)/(y)
//Divide x by n: 	x / n
#define DIV_FP_INT(x,n) (x)/(n)
/* == Implementation */

/* States in a thread's life cycle. */
enum thread_status
  {
    THREAD_RUNNING,     /* Running thread. */
    THREAD_READY,       /* Not running but ready to run. */
    THREAD_BLOCKED,     /* Waiting for an event to trigger. */
    THREAD_DYING        /* About to be destroyed. */
  };

/* Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) -1)          /* Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0                       /* Lowest priority. */
#define PRI_DEFAULT 31                  /* Default priority. */
#define PRI_MAX 63                      /* Highest priority. */

/* A kernel thread or user process.

   Each thread structure is stored in its own 4 kB page.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
        0 kB +---------------------------------+

   The upshot of this is twofold:

      1. First, `struct thread' must not be allowed to grow too
         big.  If it does, then there will not be enough room for
         the kernel stack.  Our base `struct thread' is only a
         few bytes in size.  It probably should stay well under 1
         kB.

      2. Second, kernel stacks must not be allowed to grow too
         large.  If a stack overflows, it will corrupt the thread
         state.  Thus, kernel functions should not allocate large
         structures or arrays as non-static local variables.  Use
         dynamic allocation with malloc() or palloc_get_page()
         instead.

   The first symptom of either of these problems will probably be
   an assertion failure in thread_current(), which checks that
   the `magic' member of the running thread's `struct thread' is
   set to THREAD_MAGIC.  Stack overflow will normally change this
   value, triggering the assertion. */
/* The `elem' member has a dual purpose.  It can be an element in
   the run queue (thread.c), or it can be an element in a
   semaphore wait list (synch.c).  It can be used these two ways
   only because they are mutually exclusive: only a thread in the
   ready state is on the run queue, whereas only a thread in the
   blocked state is on a semaphore wait list. */
struct thread
  {
    /* Owned by thread.c. */
    tid_t tid;                          /* Thread identifier. */
    enum thread_status status;          /* Thread state. */
    char name[16];                      /* Name (for debugging purposes). */
    uint8_t *stack;                     /* Saved stack pointer. */
    int priority;                       /* Priority. */ /*original priority when created */
    struct list_elem allelem;           /* List element for all threads list. */
    /* my Implementation */
    //for Alarm clock
    int64_t wake_time;                  /* Saves the wake time for the thread */
    //for priority inversion and donation
    int donated_priority;               /* Saves the priority donated from another thread during thread donation */
    int second_donated_priority;        /* second donated priority set  */
    bool is_donated;                    /* check whether the thread has been donated priority or not;false by default   */
    bool is_second_donated;             /* checks whether the thread is donated again */

    //multilevel priority scheduler
    int nice;                           /* stores the nice value of each thread */
    int recent_cpu;                     /* stores the CPU time of each thread */
    /*==== My Implementaion */

    /* Shared between thread.c and synch.c. */
    struct list_elem elem;              /* List element. */

#ifdef USERPROG
    /* Owned by userprog/process.c. */
    uint32_t *pagedir;                  /* Page directory. */
#endif

    /* Owned by thread.c. */
    unsigned magic;                     /* Detects stack overflow. */
  };

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;

void thread_init (void);
void thread_start (void);

void thread_tick (void);
void thread_print_stats (void);

typedef void thread_func (void *aux);
tid_t thread_create (const char *name, int priority, thread_func *, void *);

void thread_block (void);
void thread_unblock (struct thread *);

struct thread *thread_current (void);
tid_t thread_tid (void);
const char *thread_name (void);

void thread_exit (void) NO_RETURN;
void thread_yield (void);

/* Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func (struct thread *t, void *aux);
void thread_foreach (thread_action_func *, void *);

int thread_get_priority (void);
void thread_set_priority (int);

int thread_get_nice (void);
void thread_set_nice (int);
int thread_get_recent_cpu (void);
int thread_get_load_avg (void);

/* my Implementation */

bool compare_by_priority(const struct list_elem *_a, const struct list_elem *_b, void *aux UNUSED);
bool compare_by_donated_priority(const struct list_elem *_a, const struct list_elem *_b, void *aux UNUSED);
void thread_yield_cur (struct thread *cur);
/* Made it to extern for it to be accessby other files. same as in thread.c*/
extern struct list ready_list;
/* List of all processes.  Processes are added to this list
   when they are first scheduled and removed when they exit. */
extern struct list all_list;

/* mlfqs implementation */
void refresh_load_avg(void)   ;  /*function to update the load average */
int refresh_priority(struct thread* a); /* refreshing the priority of the threads */
int refresh_priority_cur(void); /* refreshing the priority of the threads */
int refresh_recent_cpu(struct thread* t);  /* Refeshes the recent CPU of thread t*/
/*=== My Implementation */

#endif /* threads/thread.h */

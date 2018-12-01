#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>

struct scheduler;

typedef void (*taskfunc)(void*, struct scheduler *);

typedef struct tache tache;
struct tache{
    taskfunc f;
    void *closure;
    tache *suivant;
};

typedef struct lifo lifo;
struct lifo{
    tache *premier;
};

typedef struct Task{
    taskfunc func;
    void *arg;
}Task;

typedef struct Element{
  Task * task;
  struct Element *next;
  struct Element *previous;
} Element;

typedef struct Deque{
  pthread_t thread;
  Element *head;
  Element *tail;
} Deque;

void empiler(lifo *pile, taskfunc f, void *closure);

tache *depiler(lifo *pile);

void *tache_todo(void *arg);

Deque *init(void);

void deque_push_front(Deque *deque, Task* task);

void deque_push_back(Deque *deque, Task* task);

Task* deque_pop_front(Deque *deque);

Task* deque_pop_back(Deque *deque);

int idThreadsDeq(struct scheduler *s);

static inline int
sched_default_threads()
{
    return sysconf(_SC_NPROCESSORS_ONLN);
}

int sched_init(int nthreads, int qlen, taskfunc f, void *closure);
int sched_spawn(taskfunc f, void *closure, struct scheduler *s);


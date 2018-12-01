#include "sched.h"

struct scheduler{
 // pthread_t *threads;
  pthread_mutex_t mutex;
  pthread_mutex_t mutex_cpt;
  int cpt_mutex;
  Deque **deq;
  int nbrethreads;
  int len;
};

Deque *init(void){
  Deque *p_new = malloc(sizeof*p_new);
  if(p_new != NULL){
    p_new->head = NULL;
    p_new->tail = NULL;
  }
  return p_new;
}

void deque_push_back(Deque *deque, Task* task){

  Element *e = malloc(sizeof(Element));
  assert(e != NULL);
  e->task = task;
  e->previous = deque->tail;
  e->next = NULL;
  if(deque->head == NULL){
    deque->head = deque->tail = e;
  }
  else{
    deque->tail->next = e;
    deque->tail = e;
  }
  
}

void deque_push_front(Deque *deque, Task* task){
  Element *e = malloc(sizeof(Element));
  assert(e != NULL);
  e->task = task;
  e->next = deque->head;
  e->previous = NULL;
  if(deque->tail == NULL){
    deque->head = deque->tail = e;

  }
  else{
    deque->head->previous = e;
    deque->head = e;
  }

}
	
void destroy(Deque *deque){
 	free(deque->head);
  free(deque->tail);
}


Task* deque_pop_back(Deque *deque){
  if(deque->tail == NULL)
    return NULL;
  Task *func = deque->tail->task;
  Element *e = deque->tail;
  if(deque->head == deque->tail)
    deque->head = deque->tail = NULL;
  else
    deque->tail = e->previous;
  free(e);
  
  return func;
}


Task* deque_pop_front(Deque *deque){
  if(deque->head == NULL)
    return NULL;

  Task *func = deque->head->task;
  Element *e = deque->head;
  if(deque->head == deque->tail)
    deque->head = deque->tail = NULL;
  else
    deque->head = e->next;
  free(e);
  
  return func;
}


int idThreadsDeq(struct scheduler *s){
  pthread_t thread = pthread_self();
  for (int i = 0; i < s->nbrethreads; ++i){
    if(pthread_equal(thread, s->deq[i]->thread))
      return i;
  }
    return -1;
}

void *tache_todo(void *arg){
  struct scheduler *s = (struct scheduler *) arg;
  int id_thread = idThreadsDeq(s);

  while(1){
    pthread_mutex_lock(&s->mutex);

    Task *t = deque_pop_back(s->deq[id_thread]);
    if(t != NULL){
      pthread_mutex_unlock(&s->mutex);

      pthread_mutex_lock(&s->mutex_cpt);
      s->cpt_mutex++;
      pthread_mutex_unlock(&s->mutex_cpt);

      taskfunc f = t->func;
      (*f) (t->arg, s);
      pthread_mutex_lock(&s->mutex_cpt);
      s->cpt_mutex--;

      pthread_mutex_unlock(&s->mutex_cpt);

    }
    else{
      pthread_mutex_unlock(&s->mutex);

      int k = rand()%s->nbrethreads;
      pthread_mutex_lock(&s->mutex);

      int i;
      for(i = 0; i < s->nbrethreads && t == NULL; i++){
        t = deque_pop_front(s->deq[(k+i)%s->nbrethreads]);
      }
      if(t != NULL){
        pthread_mutex_unlock(&s->mutex);

        pthread_mutex_lock(&s->mutex_cpt);
        s->cpt_mutex++;
        pthread_mutex_unlock(&s->mutex_cpt);

        taskfunc f = t->func;
        (*f) (t->arg, s);

        pthread_mutex_lock(&s->mutex_cpt);
        s->cpt_mutex--;
        pthread_mutex_unlock(&s->mutex_cpt);
      }
      else{
        pthread_mutex_unlock(&s->mutex);

        if(s->cpt_mutex==0)
          break;
        usleep(1000);
      }     
    }
        
  }
  destroy(s->deq[id_thread]);
  pthread_exit(NULL);

}

int sched_init(int nthreads, int qlen, taskfunc f, void *closure){
  struct scheduler *s = malloc(sizeof(struct scheduler));
  pthread_mutex_init(&s->mutex_cpt, NULL);
  pthread_mutex_init(&s->mutex, NULL); 
  s->len = qlen;
  s->cpt_mutex = 0;
  Task *t = malloc(sizeof(Task));
  t->func = f;
  t->arg = closure;
  if(nthreads <= 0){
    s->nbrethreads = sched_default_threads();
  }
  else{
    s->nbrethreads = nthreads;
  }

  s->deq = calloc(s->nbrethreads,sizeof(Deque));
  int i;
  for(i=0; i<s->nbrethreads; i++){
    s->deq[i] = init();
  }
  deque_push_front(s->deq[0], t);

  for(i = 0; i<s->nbrethreads; i++){
   
    
    pthread_create(&s->deq[i]->thread, NULL, tache_todo, s);

  }
  for (int i = 0; i < s->nbrethreads; i++){
    pthread_join(s->deq[i]->thread, NULL);
  }

  return 1;
}


int sched_spawn(taskfunc f, void *closure, struct scheduler *s){
  Task *t = malloc(sizeof(Task));
  t->func = f;
  t->arg = closure;
  pthread_mutex_lock(&s->mutex); 
  int id_thread = idThreadsDeq(s);
     
  deque_push_back(s->deq[id_thread], t);
  
  pthread_mutex_unlock(&s->mutex);

  return 1;
}






#include "sched.h"

struct scheduler{
    lifo *pile;
    pthread_t* threads;
    pthread_mutex_t mutex;
    pthread_mutex_t mutex_cpt;
    int cpt_mutex;
    int nbrethreads;
    int len;
};



void empiler(lifo *pile, taskfunc f, void *closure){

    tache *t = malloc(sizeof(tache));
    if(pile != NULL || t == NULL){
    	t->f = f;
    	t->closure = closure;
    	t->suivant = pile->premier;
    	pile->premier = t;
	}
}

tache *depiler(lifo *pile){
	if(pile != NULL && pile->premier != NULL){
		tache *t = pile->premier;
		pile->premier = t->suivant;
		return t;
	}

    return NULL;
}

void *tache_todo(void *arg){
    struct scheduler *s = (struct scheduler *) arg;
    tache *t = NULL;

    while (1) {
    
      pthread_mutex_lock(&s->mutex);
      t = depiler(s->pile);
      pthread_mutex_unlock(&s->mutex);
      if(t!=NULL){
       	pthread_mutex_lock(&s->mutex_cpt);
       	s->cpt_mutex++;
    		pthread_mutex_unlock(&s->mutex_cpt);   	
        taskfunc f = t->f;
    		(*f) (t->closure, s);
     		pthread_mutex_lock(&s->mutex_cpt);
  			s->cpt_mutex--;
   			pthread_mutex_unlock(&s->mutex_cpt);
      }
      if(s->pile->premier == NULL && s->cpt_mutex == 0) /*si toutes taches ont ete executés et si tous processus fini(ou en etat de fin) */
        break; 
    } 
    pthread_exit(NULL);
}

int sched_init(int nthreads, int qlen, taskfunc f, void *closure){
	struct scheduler *s = malloc(sizeof(struct scheduler));
  pthread_mutex_init(&s->mutex_cpt, NULL);
  pthread_mutex_init(&s->mutex, NULL);
  s->cpt_mutex = 0;
 
	s->pile = malloc(sizeof(s->pile));
	s->len = qlen;

	empiler(s->pile, f, closure);
	if(nthreads <= 0){
		s->nbrethreads = sched_default_threads();
	}
	else{
		s->nbrethreads = nthreads;
	}

	s->threads = malloc(s->nbrethreads * sizeof(s->threads));
	int i;
	for (i = 0; i < s->nbrethreads; i++){
		if(pthread_create(&s->threads[i], NULL, tache_todo, s)){
			perror("pthread_create");
		}
	}
	for (int i = 0; i < s->nbrethreads; i++){
		pthread_join(s->threads[i], NULL);
		
	}
	

	return 1;

}

int sched_spawn(taskfunc f, void *closure, struct scheduler *s){
   
    pthread_mutex_lock(&s->mutex); 
    empiler(s->pile, f, closure);
    pthread_mutex_unlock(&s->mutex); 

    return 1;
}


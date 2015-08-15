#ifndef TFAL_TIMER_H
#define TFAL_TIMER_H

#include <signal.h>
#include <time.h>

#include "rb_tree.h"
//#define MULTI_THREAD
/*
 * application layer timer - without class
 * - single-thread version -
 */

#define TFAL_SIGNO SIGUSR1

struct ap_timerset{
	
	timer_t internal_timer;
	struct sigaction oldact;
//rbtree
	struct rb_tree rbtree;
	//pthread_spinlock_t rbt_lock;

	_key_t tick;
	int timer_num;
	int (*add_timer) (int ms, ap_callback fn, void *arg, ap_key* ret);//success - 0, fail - -1 
	void (*delete_timer) (ap_key* key);//same expire but fn not same
	void (*delete_all) ();
};


extern void ap_initial(struct ap_timerset *t);//t->add_timer = ...

#endif
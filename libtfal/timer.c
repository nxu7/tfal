#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "timer.h"

#define errExit(msg) do { perror(msg); exit(EXIT_FAILURE); } while(0)
#define errExit_En(en,msg)  do { errno = en; perror(msg); exit(EXIT_FAILURE); } while(0)

int add_timer(int ms, ap_callback fn, void *arg, ap_key* ret);
void delete_timer(ap_key* key);
void delete_all();

struct ap_timerset *this_ap;

void _tick(int signum, siginfo_t* siginfo, void* ucontext) 
{
	timer_t *ti;
	int ret;
	ti = siginfo->si_value.sival_ptr;
	ret = timer_getoverrun(*ti);
	if(-1 == ret){
		errExit("timer_getoverrun");
	}
	/*
	 * ret is 0 - no tick miss before
	 *        >0 - miss [ret] ticks
	 */
	
	this_ap->rbtree.rb_del_in_range(this_ap->tick + ret + 1);
	// run the timer whose expire is <= to

	this_ap->tick += ret + 1;
}

void _sig_blk_op(int how)
{
	assert(how == SIG_BLOCK || how == SIG_UNBLOCK);
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, TFAL_SIGNO);	/* local_var ?*/
#ifdef MULTI_THREAD
	if(0 != (ret = pthread_sigmask(how, &maske, NULL))){
		errExit_En(ret, "pthread_sigmask");
	}
#else
	if(-1 == sigprocmask(how, &mask, NULL)){	//only for single thread version 
		errExit("sigprocmask");
	}
#endif
}


void _setick(timer_t *internal, struct sigaction *oldac)
{
	struct sigevent se;
	struct sigaction sa;
	struct itimerspec its;

	//setup signal handler
	sigemptyset(&(sa.sa_mask));
	sa.sa_sigaction = &_tick;
	sa.sa_flags = SA_SIGINFO;
	if(sigaction(TFAL_SIGNO, &sa, oldac) < 0){	
		errExit("sigaction");
	}

	//setup the way internal timer notify
	se.sigev_notify = SIGEV_SIGNAL;
	se.sigev_signo = TFAL_SIGNO;
	se.sigev_value.sival_ptr = internal;
	if(-1 == timer_create(CLOCK_MONOTONIC, &se, internal)){//CLOCK_MONOTONIC - timer won't be disturb by NTP
		errExit("timer_create");
	}

	//start the internal timer - "ring" every 0.1ms
	its.it_interval.tv_sec = its.it_value.tv_sec = 0; 
	its.it_interval.tv_nsec = its.it_value.tv_nsec = 100000;//0.1ms

	if(-1 == timer_settime(*internal, 0, &its, NULL)){//start ticking
		errExit("timer_settime");
	}
}

void _stoptick(timer_t *internal, struct sigaction *oldac)
{	

	if(sigaction(TFAL_SIGNO, oldac, NULL) < 0){	//recover the sig_action
		errExit("sigaction");
	}
	struct itimerspec its;
	its.it_interval.tv_sec = its.it_value.tv_sec = 0; 
	its.it_interval.tv_nsec = its.it_value.tv_nsec = 0;

	if(-1 == timer_settime(*internal, 0, &its, NULL)){//stop ticking
		errExit("timer_settime");
	}
}


void ap_initial(struct ap_timerset *t)
{
	t->add_timer = &add_timer;
	t->delete_timer = &delete_timer;
	t->delete_all = &delete_all;

	t->timer_num = 0;
	t->tick = 0;


	this_ap = t;//this_ap: global

	//rbtree initial
	rb_initial(&(t->rbtree));
}


/*
 * add a timer which expire at @ms ms later.
 * 
 * @return 0 on success
 *         -1 if there are already 32 timers on the same expire.
 */
int add_timer(int ms, ap_callback fn, void *arg, ap_key* retval)//if multithread, need to add sth. like 'lock'
{
	assert(ms > 0);//a legal timer
	_sig_blk_op(SIG_BLOCK);
	int ret;
	if(this_ap->timer_num == 0){ //if add first timer start ticking 
		_setick(&(this_ap->internal_timer), &(this_ap->oldact));
	}
	this_ap->timer_num++;

	_key_t expire = this_ap->tick + ms * 10;
	
	if(-1 == (ret = this_ap->rbtree.rb_insert(expire, fn, arg, retval))){
		printf("EACH_NODE_FNUM FULL!\n");
	}

	_sig_blk_op(SIG_UNBLOCK);

	return ret;
}

/*
 * delete one timer
 * 
 */
void delete_timer(ap_key* key)
{
	assert(key != NULL);
	_sig_blk_op(SIG_BLOCK);
	this_ap->rbtree.rb_delete(key, DEL_ONLY);
	this_ap->timer_num--;
	if(this_ap->timer_num == 0){
		_stoptick(&(this_ap->internal_timer), &(this_ap->oldact));
	}
	_sig_blk_op(SIG_UNBLOCK);
}

/*
 * delete all timers
 *
 */

void delete_all()
{
	_sig_blk_op(SIG_BLOCK);
	_stoptick(&(this_ap->internal_timer), &(this_ap->oldact));//stop ticking
	_sig_blk_op(SIG_UNBLOCK);
	this_ap->timer_num = 0;
	this_ap->rbtree.rb_destroy();
}

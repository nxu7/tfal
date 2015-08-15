//single thread test
//
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "./libtfal/timer.h"

int TESTNUM = 0;
int RANDOMNUM = 0;
int TIMEOUT = 0;
/*return now - last*/
void _lasthowlong(struct timeval *last, struct timeval *now,struct timeval *hl)
{
	if(hl){
		hl->tv_sec = now->tv_sec - last->tv_sec;
		hl->tv_usec = now->tv_usec - last->tv_usec;
		if(hl->tv_usec < 0){
			hl->tv_sec--;
			hl->tv_usec += 1000000L;
		}
	}
}

long _ms(struct timeval *x)
{
	long tmp = x->tv_sec * 1000;
	tmp += x->tv_usec / 1000;
	return tmp;
}

struct ap_timerset test;
int k = 0;

void callback1(void *arg)
{
	//printf("callback :%d \n",*(int*)arg);
	if(k < TESTNUM){
		test.add_timer(TIMEOUT, callback1, NULL, NULL);
		k++;
	}
}


int rn = 0;
void callback_random(void *arg)
{
	//do nothing;
	rn++;
	if(rn % 100)
		printf("random timer\n");
}

/*
 * test case 1
 */

int test1()//test for accuracy. 
{
	ap_initial(&test);

	struct timeval st,ed,hl;
	gettimeofday(&st,NULL);

	test.add_timer(TIMEOUT, callback1, NULL, NULL);
	while(1){
		//busy do sth
		if(k >= TESTNUM){
			break;
		}
	}
	gettimeofday(&ed,NULL);
	_lasthowlong(&st, &ed, &hl);
	printf("howlong %ld ms, calltimes %d \n",_ms(&hl), k);

	test.delete_all();
	return 0;
}


void randomadd(void *args)
{
	srand((unsigned)time(NULL));
	int timeout;
	int i;
	for(i = 0; i < RANDOMNUM; i++){
		timeout = (rand() % 10000 ) + 1;
		if(timeout > 0)
			test.add_timer(timeout , callback_random, NULL, NULL);
	}
}

/*
 * test case 2
 * 
 */

int test2()
{
	struct timeval st,ed,hl;
	ap_initial(&test);

	randomadd(NULL);//add some random timer

	gettimeofday(&st,NULL);

	test.add_timer(TIMEOUT, callback1, NULL, NULL);
	while(1){
		//busy do sth
		if(k >= TESTNUM){
			break;
		}
	}
	gettimeofday(&ed,NULL);

	_lasthowlong(&st, &ed, &hl);
	printf("howlong %ld ms, calltimes %d \n",_ms(&hl), k);
	
	return 0;
}

/*
 * test case 3
 * 
 */
int test3()
{

}




void help()
{
	printf("command:\t./a 1 [TESTNUM] [TIMEOUT]\n\
		./a 2 [TESTNUM] [TIMEOUT] [RANDOM]\n\
		./a 3\n");
}


int main(int argc, char *argv[])//a 
{
	if(argc < 2){
		help();
		return 0;
	}
	switch(atoi(argv[1])){
		case 1:
			TESTNUM = atoi(argv[2]);//command - ./a 1 [TESTNUM] [TIMEOUT]
			TIMEOUT = atoi(argv[3]);
			test1();
			break;
		case 2:
			TESTNUM = atoi(argv[2]);//command - ./a 2 [TESTNUM] [TIMEOUT] [RANDOM]
			TIMEOUT = atoi(argv[3]);
			RANDOMNUM = atoi(argv[4]);
			test2();
			break;
		case 3:
			//command - ./a 3
			break;
	}

	return 0;
}
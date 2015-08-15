#include <stdio.h>
#include <limits.h>
#define EACH_NODE_FNUM 32 
/*
 * 
 * e.g.
 * 0x00000400 --> 10
 */
int _ntz(unsigned int x)
{
	int b4,b3,b2,b1,b0;
	b4 = (x & 0x0000ffff)? 0:16;
	b3 = (x & 0x00ff00ff)? 0:8;
	b2 = (x & 0x0f0f0f0f)? 0:4;
	b1 = (x & 0x33333333)? 0:2;
	b0 = (x & 0x55555555)? 0:1;
	return b0 + b1 + b2 + b3 + b4;
}

/*
 *
 *	get an id to use
 */
int _getidx(unsigned int mask)
{
	if(~mask == 0){
		return -1;//FULL
	}
	unsigned int x = mask | (mask + 1);
	x = x & (~mask);
/*
	if(x >> 16){ x = x >> 16; id += 16; }
	if(x >> 8){ x = x >> 8; id += 8; }
	if(x >> 4){ x = x >> 4; id += 4; }
	id += (x >> 1) - (x >> 3);
*/
	return _ntz(x);
}
/*
 *
 * iterate mask's 1_bits
 * 
 */
void _itr_mask(unsigned int mask)
{
	unsigned int x;
	while(mask != 0){
		x = mask & (mask - 1);//first bit 1 ==> 0
		x = (~x) & mask;
		printf("%d %x\n",_ntz(x), mask);
		mask &= (~x);
	}
}



int testmasks(){
	unsigned int xmask = ~0;
	int i,id;
/*
	for(i = 0; i < EACH_NODE_FNUM; i++){
		xmask = 0;
		xmask |= 1<<i;
		printf("%d\n",_ntz(xmask));
	}
*/

	//for(i = 0; i < EACH_NODE_FNUM; i++){
		xmask = 8984511;
		printf("%x\n\n",xmask);
		//id = _getidx(xmask);
		//xmask |= (1<<id);
		//printf("%x %d\n",xmask, id);
		_itr_mask(xmask);
	//}

}

int testick()
{
	int tick = INT_MAX;
	++tick;
	printf("%d %d \n",tick, tick + 1);

}
int testick2()
{
	unsigned long long x1 = 0,x2 = 0;
	if(x1 + 1 > x2){
		printf("ok %d\n", sizeof(x1));
	}
	return 0;
}

int main()
{
	testick2();
	return 0;
}

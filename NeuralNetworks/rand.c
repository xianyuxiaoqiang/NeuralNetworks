#include "common.h"

int inited = 0;
int initRand()
{
	if( inited == 0 )
	{
		srand((int)time(0));
		inited = 1;
	}
	return 0;
}

int getRand(int mod)
{
	initRand();
	return rand() % mod;
}

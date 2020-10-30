/***********************************************************
 * Program name: cache.c
 * Description: A program that simulates a cache. Further 
 *				information in report.docx
 * Author: F74076132
 * ********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define INF 0x80000000

/*static variables*/
int CACHE_SIZE, BLOCK_SIZE, ASSOCIATIVITY, REPLACE_MODE;
int SET_AMOUNT, BLOCKS_PER_SET;
int TAG_LEN, INDEX_LEN, OFFSET_LEN;
unsigned int TAG, INDEX;
unsigned int** CACHE_ARRAY;//implementation of cache

/*function declarations*/
void decode(unsigned int);
void check(int*, int*);
unsigned int decide();
unsigned int FIFO(int, int);
unsigned int LRU(int, int);
unsigned int MRU(int, int);

int main(int argc, char* argv[]){
	FILE *FILE_IN, *FILE_OUT;
	FILE_IN = fopen(argv[1], "r");
	FILE_OUT = fopen(argv[2], "w");
	
	if(FILE_IN == NULL || FILE_OUT == NULL){
		printf("IO error, program ends!\n");
		return -1;
	}

	//read file
	fscanf(FILE_IN, "%d %d %d %d ", &CACHE_SIZE, &BLOCK_SIZE, &ASSOCIATIVITY, &REPLACE_MODE);
	CACHE_SIZE *= 1024;//kb to byte
	//printf("%d %d %d %d\n", CACHE_SIZE, BLOCK_SIZE, ASSOCIATIVITY, REPLACE_MODE);
	
	//determine some parameters
	switch(ASSOCIATIVITY){
		case 0:
			//directed-mapped
			SET_AMOUNT = CACHE_SIZE / BLOCK_SIZE;
			BLOCKS_PER_SET = 1;
			break;
		case 1:
			//4-way set-associative
			SET_AMOUNT = (CACHE_SIZE / 4) / BLOCK_SIZE;
			BLOCKS_PER_SET = 4;
			break;
		case 2:
			//fully associative 
			SET_AMOUNT = 1;
			BLOCKS_PER_SET = CACHE_SIZE / BLOCK_SIZE;
			break;
		default:
			printf("wrong input!(associativity bit)\n");
			exit(-1);
			break;	
	}
	OFFSET_LEN = (int)log2(BLOCK_SIZE);
	INDEX_LEN = (int)log2(SET_AMOUNT);
	TAG_LEN = 32 - INDEX_LEN - OFFSET_LEN;

	//memory setup
	CACHE_ARRAY = (unsigned int**)malloc(SET_AMOUNT*sizeof(unsigned int*));
	unsigned int* pEntry = (unsigned int*)malloc(SET_AMOUNT*BLOCKS_PER_SET*sizeof(unsigned int));
	for(int i=0; i<SET_AMOUNT; ++i, pEntry += BLOCKS_PER_SET){
		CACHE_ARRAY[i] = pEntry;
	}

	//initialize
	for(int i=0; i<SET_AMOUNT; ++i){
		for(int j=0; j<BLOCKS_PER_SET; ++j){
			CACHE_ARRAY[i][j] = INF;
		}
	}	
	
	//reading things to run
	unsigned int addr;
	while(fscanf(FILE_IN, "%x", &addr) != EOF){
		//printf("%x\n", addr);
		decode(addr);
		unsigned int a = decide();
		fprintf(FILE_OUT, "%d\n", a);	
		//printf("%d\n", a);
	}
	
	//destroy
	free(CACHE_ARRAY[0]);
	free(CACHE_ARRAY);

	fclose(FILE_IN);
	fclose(FILE_OUT);

	return 0;	
}


/***********************************************************
 * Function: decode
 * Description: Decode the access, find the TAG bit and 
 *				INDEX bit (store in static variables TAG &
 *				INDEX) 
 * Parameter: unsigned int addr (a 32-bit space)
 * Return: none
 * ********************************************************/
void decode(unsigned int addr){
	unsigned int cpy = addr;
	//calculate TAG by right shift (index length + oddset length) times
	for(int i=0; i<(INDEX_LEN + OFFSET_LEN); ++i){
		cpy >>= 1;		
	}
	TAG = cpy;
	
	//calculate INDEX by left shift tag length times to eliminate tag, then right shift (tag length + offset length) time
	cpy = addr;
	for(int i=0; i<TAG_LEN; ++i){
		cpy <<= 1;
	}
	for(int i=0; i<(TAG_LEN + OFFSET_LEN); ++i){
		cpy >>= 1;
	}
	INDEX = cpy;

	//printf("addr:0x%x => T:%d (0x%x), I:%d (0x%x)\n", addr, TAG, TAG, INDEX, INDEX);
}


/***********************************************************
 * Function: decide
 * Description: Decide the movements that should be done.
 *				And find the victim.
 * Parameter: none
 * Return: an unsigned int, which present the victim
 *			(-1 for no victim)
 * ********************************************************/
unsigned int decide(){
	int hitCode, hitIndex;
	check(&hitCode, &hitIndex);
	unsigned int tmp;	
	switch(REPLACE_MODE){
		case 0:
			//FIFO
			return FIFO(hitCode, hitIndex);
			break;
		case 1:
			//LRU
			return LRU(hitCode, hitIndex);
			break;
		case 2:
			//
			return MRU(hitCode, hitIndex);
			break;
		default:
			printf("wrong input!(replace algorithm)\n");
			exit(-1);
			break;	
	}
}


/***********************************************************
 * Function: check
 * Description: Calculate the value of hitCode & hitIndex.
 *				hitCode: 0 for conflict/capacity miss,
 *						 1 for compulsory miss, 
 *						 2 for hit
 *				hitIndex: where the block should be (index
 *						  in the set)
 * Parameter: int *h for hitCode, *k for hitIndex
 * Return: none (use pointer parameter for pass by ref.)
 * ********************************************************/
void check(int* hitCode, int *hitIndex){
	//0 : conflict/capacity miss, 1 : compulsory miss, 2 : hit
	*hitCode = 0;
	*hitIndex = 0;
	for(int i=0; i<BLOCKS_PER_SET; ++i){
		if(TAG == CACHE_ARRAY[INDEX][i]){
			*hitCode = 2;
			*hitIndex = i;
			return;
		}
		else if(CACHE_ARRAY[INDEX][i] == INF){
			*hitCode = 1;
			*hitIndex = i;
			return;
		}
		else
			continue;
	}
}


/***********************************************************
 * Function: FIFO
 * Description: Move every block backwards, and add the new
 *				block to the top of the set. This means the 
 *				block at the end of the set is the victim.
 * Parameter: int h for hitCode, k for hitIndex
 * Return: an unsigned int, which present the victim
 *			(-1 for no victim)
 * ********************************************************/
unsigned int FIFO(int h, int k){
	if(h == 2){
	   	return -1;
	}
	if(h == 1){
		CACHE_ARRAY[INDEX][k] = TAG;
		return -1;
	}

	unsigned int tmp = CACHE_ARRAY[INDEX][0];
	for(int i=1; i<BLOCKS_PER_SET; ++i){
		CACHE_ARRAY[INDEX][i-1] = CACHE_ARRAY[INDEX][i];
	}
	CACHE_ARRAY[INDEX][BLOCKS_PER_SET-1] = TAG;
	return tmp;
}


/***********************************************************
 * Function: LRU
 * Description: Use the set itself for the queue which 
 *				performs like a min-heap to implement the
 *				LRU replace algorithm.
 * Parameter: int h for hitCode, k for hitIndex
 * Return: an unsigned int, which present the victim
 *			(-1 for no victim)
 * ********************************************************/
unsigned int LRU(int h, int k){
	if(h == 2){
		//heapify the queue(set)
		unsigned int tmp = CACHE_ARRAY[INDEX][k];
		for(int i=k; i>=1; --i){
			CACHE_ARRAY[INDEX][i] = CACHE_ARRAY[INDEX][i-1];
		}
		CACHE_ARRAY[INDEX][0] = tmp;
		return -1;
	}
	if(h == 1){
		//add to top
		for(int i=k; i>=1; --i){
			CACHE_ARRAY[INDEX][i] = CACHE_ARRAY[INDEX][i-1];
		}
		CACHE_ARRAY[INDEX][0] = TAG;
		return -1;
	}
	
	unsigned int tmp = CACHE_ARRAY[INDEX][BLOCKS_PER_SET-1];
	for(int i=BLOCKS_PER_SET-1; i>=1; --i){
		CACHE_ARRAY[INDEX][i] = CACHE_ARRAY[INDEX][i-1];
	}
	CACHE_ARRAY[INDEX][0] = TAG;
	return tmp;
}


/***********************************************************
 * Function: MRU
 * Description: Kick the most recent used block. (Reverse 
 *				version of LRU)
 * Parameter: int h for hitCode, k for hitIndex
 * Return: an unsigned int, which present the victim
 *			(-1 for no victim)
 * ********************************************************/
unsigned int MRU(int h, int k){
	if(h == 2){
		unsigned int tmp = CACHE_ARRAY[INDEX][k];
		for(int i=k; i>=1; --i){
			CACHE_ARRAY[INDEX][i] = CACHE_ARRAY[INDEX][i-1];
		}
		CACHE_ARRAY[INDEX][0] = tmp;
		return -1;
	}
	if(h == 1){
		//add to top
		for(int i=k; i>=1; --i){
			CACHE_ARRAY[INDEX][i] = CACHE_ARRAY[INDEX][i-1];
		}
		CACHE_ARRAY[INDEX][0] = TAG;
		return -1;
	}
	
	unsigned int tmp = CACHE_ARRAY[INDEX][0];
	CACHE_ARRAY[INDEX][0] = TAG;
	return tmp;	
}	

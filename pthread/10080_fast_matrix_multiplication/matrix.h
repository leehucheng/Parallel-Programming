#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
 
#define MAX_THREAD 20
#define UINT unsigned long
#define MAXN 2048
 
UINT tmp[MAXN][MAXN];
typedef struct MulArg {
    int tid, up, down, N;
    UINT (*A)[MAXN];
    UINT (*B)[MAXN];
    UINT (*C)[MAXN];
} MulArg;
 
void multiply(int N, unsigned long A[][2048], unsigned long B[][2048], unsigned long C[][2048]);

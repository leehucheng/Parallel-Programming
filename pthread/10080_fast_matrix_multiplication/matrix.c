#include "matrix.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
 
#define MAX_THREAD 20
#define UINT unsigned long
#define MAXN 2048
 
UINT tmp[MAXN][MAXN];
UINT result[MAXN][MAXN];
 
void *mul_split(void *arg){
    MulArg *mult = (MulArg *) arg;
 
    if(mult->N <= 20){
        for (int j = 0; j < mult->N; j++)
        {
            unsigned long sum = 0;    // overflow, let it go.
            for (int k = 0; k < mult->N; k++)
                sum += mult->A[mult->tid][k] * mult->B[j][k];
            result[mult->tid][j] = sum;
        }
    }
    else{
        for (int i = mult->up ; i <= mult->down; i++)
        {
            for (int j = 0; j < mult->N; j++)
            {
                unsigned long sum = 0;    // overflow, let it go.
                for (int k = 0; k < mult->N; k++)
                    sum += mult->A[i][k] * mult->B[j][k];
                result[i][j] = sum;
            }
        }
    }
 
//    printf("thread[%d] is done. \n", mult->tid);
    pthread_exit(NULL);
 
 
}
 
void multiply(int N, unsigned long A[][2048], unsigned long B[][2048], unsigned long C[][2048]) {
 
    for (int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            tmp[i][j] = B[i][j];
//            printf("%d ", B[j][i]);
        }
//        printf("\n");
    }
    for (int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            B[j][i] = tmp[i][j];
//            printf("%d ", B[j][i]);
        }
//        printf("\n");
    }
    int max = MAX_THREAD;
 
    if ( N < 20 ){
        max = N;
//        printf("max = %d \n", max);
    }
 
    pthread_t threads[max];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
 
    for (int t = 0; t < max ; t++){
        MulArg *data = (MulArg *) malloc(sizeof(MulArg));
        data->up = t * N / max;
        data->down = ((t+1) * N / max) - 1;
        data->A = A;
        data->B = B;
        data->C = C;
        data->N = N;
        data->tid = t;
        pthread_create(&threads[t], NULL, mul_split, (void *) data);
    }
 
    for (int i = 0; i < max ; i++)
        pthread_join(threads[i], NULL);
 
/*    for (int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            printf("%d ", A[i][j]);
        }
        printf("\n");
    }
    for (int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            printf("%d ", B[i][j]);
        }
        printf("\n");
    }*/
 
    for (int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            C[i][j] = result[i][j];
//            printf("%d ", C[i][j]);
        }
//        printf("\n");
    }
 
 
//    return A, B, C;
}
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "utils.h"
#include <assert.h>
#include <pthread.h>
 
#define MAXN 10000005
#define MAX_THREAD 4
int prefix_sum[MAXN];
uint32_t key;
 
typedef struct SumArg {
    int l ;
    int r ;
    int tid;
} SumArg;
 
int l_1;
int r_1;
int l_2;
int r_2;
int l_3;
int r_3;
int l_4;
int r_4;
 
void *pre_sum(void *a){
    SumArg *arr = (SumArg*) a;
    if (arr->tid == 0){
        l_1 = arr->l;
        r_1 = arr->r;
    }
    else if (arr->tid == 1){
        l_2 = arr->l;
        r_2 = arr->r;
    }
 
    else if (arr->tid == 2){
        l_3 = arr->l;
        r_3 = arr->r;
    }
    else{
        l_4 = arr->l;
        r_4 = arr->r;
    }
 
    int sum = 0;
    for (int i = arr->l; i <= arr->r; i++){
        sum += encrypt(i, key);
        prefix_sum[i] = sum;
    }
    pthread_exit(NULL);
}
 
void *revise_sum(void *a){
    SumArg *arr = (SumArg*) a;
 
    int sum = 0;
    for (int i = arr->l; i <= ((arr->r) - 1); i++){
        prefix_sum[i] += prefix_sum[(arr->l)-1];
    }
    pthread_exit(NULL);
}
 
int main() {
    int n;
    while (scanf("%d %" PRIu32, &n, &key) == 2) {
 
        pthread_t threads[MAX_THREAD];
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
 
        prefix_sum[0] = 0;
 
        for (int j = 0; j < MAX_THREAD; j++){
            SumArg *a = (SumArg *)malloc(sizeof(SumArg));
            if (n <= 4){
                a->l = 1;
                a->r = n;
                pthread_create(&threads[0], &attr, pre_sum, (void *)a);
                break;
            }
            else{
                a->l = (j * n / MAX_THREAD ) + 1;
                a->r = ((j + 1) * n / MAX_THREAD)  ;
                a->tid = j;
                pthread_create(&threads[j], &attr, pre_sum, (void *)a);
            }
        }
 
        if (n >4){
            for (int i = 0; i < MAX_THREAD; i++) {
                pthread_join(threads[i], NULL);
            }
        }
        else{
            pthread_join(threads[0], NULL);
        }
 
        prefix_sum[r_2] += prefix_sum[r_1];
        prefix_sum[r_3] += prefix_sum[r_2];
        prefix_sum[r_4] += prefix_sum[r_3];
 
        if ( n > 4 ){
            for (int j = 1; j < MAX_THREAD; j++){
            SumArg *a = (SumArg *)malloc(sizeof(SumArg));
                a->l = (j * n / MAX_THREAD ) + 1;
                a->r = ((j + 1) * n / MAX_THREAD)  ;
                a->tid = j;
                pthread_create(&threads[j], &attr, revise_sum, (void *)a);
            }
        }
 
        if ( n > 4 ){
            for (int i = 1; i < MAX_THREAD; i++) {
                pthread_join(threads[i], NULL);
            }
        }
 
        output(prefix_sum, n);
 
    }
    pthread_exit(NULL);
    return 0;
}
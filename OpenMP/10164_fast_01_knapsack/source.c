#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <omp.h>
 
int max(int a, int b) { return (a > b)? a : b; }
 
int knapSack(int M, int N, int W[], int V[], int Kp[])
{
 
    memset(Kp, 0, sizeof(Kp[0]) * (M+1));
    for (int i = 0; i < N; i++) {
        int v = V[i], w = W[i];
        for (int j = M; j >= w; j--)
            Kp[j] = max(Kp[j], Kp[j-w]+v);
    }
 
 
/*    int *K = (int*) malloc((M+1)*sizeof(int));
    memset(Kp, 0, sizeof(Kp[0]) * (M+1) );
    long int i, m;
 
    for (i = 1; i <= N; i++)
    {
//        #pragma omp parallel
//        {
//        #pragma omp for
            int vv = V[i-1], ww = W[i-1];
            for (m = 0; m <= M; m++)
            {
                if (W[i-1] <= m)
                    K[m] = max(vv + Kp[m-ww], Kp[m]);
                else
                    K[m] = Kp[m];
            }
//        }
 
        int *tmp = Kp;
        Kp = K;
        K = tmp;
    }
 
    int result = Kp[M];
 
//    free(K);
//    free(Kp);
 
    return result;*/
}
 
 
int V[100001], W[100001];
int Kp[2][5000001];
 
int main(){
 
    omp_set_num_threads(2);
    #pragma omp parallel
    {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        for (int i = 0; i < 6; i++)
            CPU_SET(i, &cpuset);
        assert(sched_setaffinity(0, sizeof(cpuset), &cpuset) == 0);
    }
 
    int N, M;
    while(scanf("%d %d", &N, &M) == 2)
    {
        for (int i = 0; i < N; i++)
            scanf("%d %d", &W[i], &V[i]);
 //       int *Kp = (int*) malloc((M+1)*sizeof(int));
 
        memset(Kp, 0, sizeof(Kp));
 
        #pragma omp parallel sections
        {
            #pragma omp section
            knapSack(M, N/2, W, V, Kp[0]);
            #pragma omp section
            knapSack(M, N-N/2, W+N/2, V+N/2, Kp[1]);
        }
 
        int ret = 0;
        for (int i = M, j = 0, mx = 0; i >= 0; i--) {
            mx = max(mx, Kp[1][j]), j++;
            ret = max(ret, Kp[0][i] + mx);
        }
 
        printf("%d\n", ret);
    }
    return 0;
}
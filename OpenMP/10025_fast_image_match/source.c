#include <stdio.h>
#include <omp.h>
 
int main()
{
    int Ah, Aw, Bh, Bw;
    while (scanf("%d %d %d %d" , &Ah, &Aw, &Bh, &Bw) == 4) {
        int A[Ah][Aw], B[Bh][Bw];
 
        for (int i = 0; i < Ah; i++){
            for (int j = 0; j< Aw; j++){                             
                 scanf("%d" , &A[i][j]);
            }
        }
 
        for (int i = 0; i < Bh; i++){
            for (int j = 0; j< Bw; j++){                             
                 scanf("%d" , &B[i][j]);
            }
        }
 
        int final_x = Ah, final_y = Aw;
        int sum[Ah-Bh+1][Aw-Bw+1];
        int tmp = 0;
 
        omp_set_num_threads(24);
 
 #pragma omp parallel for private(tmp)
        for (int a = 0; a <= (Ah - Bh); a++ ){
            for (int b = 0; b <= (Aw - Bw); b++){              
                // count sum of diff
                for (int i = 0; i < Bh; i++){
                    for (int j = 0; j < Bw; j++){
                        tmp += (A[a+i][b+j] - B[i][j])*(A[a+i][b+j] - B[i][j]) ;
                    }
                }
                sum[a][b] = tmp;
                tmp = 0;
            }
        }
 
        // compare diff
         // (2^31)-1
        int diff = 2147483647;
        for (int a = 0; a <= (Ah - Bh); a++){
            for(int b = 0; b <= (Aw - Bw); b++){
                if( sum[a][b] < diff){
                    diff = sum[a][b];
                    final_x = a + 1;
                    final_y = b + 1;
                }
            }
        }
 
        printf("%d %d\n", final_x, final_y);
    }
    return 0;
 
}
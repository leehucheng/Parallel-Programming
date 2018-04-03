/* begin */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <omp.h>
 
#define MAXN 20
int n;
int able[20] = {};
 
int nqueen(int next, int row, int ld, int rd)
{
    if(row == (1<<n)-1)
        return 1;
 
    int position = able[next] & (1<<n)-1 & (~(row|ld|rd)) ;
    int go = 0, sum = 0;
 
    while (position != 0)
    {
        go = position & (-position);
        position -= go;
        sum += nqueen(next+1, row|go, (ld|go)<<1, (rd|go)>>1);
    }
    return sum;
 
/*    for(row = 0; row < n; row++)
    {
        if(!(rowmask & (1<<row)) & !(dia1 & (1<<(row+col))) & !(dia2 & (1<<((row+n-1)-col))) & able[0] )
        {           
            ans += nqueen(col+1,rowmask|1<<row,dia1|(1<<(row+col)), dia2|(1<<((row+n-1)-col)));
        }
    }
    return ans;*/
}
 
/* main */
int main(int argc, char *argv[])
{
  int cases = 1;
 
  while(scanf("%d", &n) == 1)
  {
    char bar[n];
    for (int i = 0; i < n; i++)
    {
        scanf("%s", bar);
        able[i] = (1<<n) - 1;
        for (int j = 0; j < n; j++)
        {
            if (bar[j] == '*')
                able[i] ^= (1<<j);
        }
    }
 
    int ans = 0, row = 0, ld = 0, rd = 0, next = 0, go = 0;
    int postion = able[0] & (1<<n)-1 & (~(row | ld | rd));
    #pragma omp parallel for schedule(static, 1) reduction(+:ans)
    for (int i = 0; i < n; i++) {
        if ( (postion>>i) & 1 )
        {
            go = 1 << i;
            int tmp = nqueen(next+1, row|go, (ld|go)<<1, (rd|go)>>1);
            ans += tmp;
        }
    }
 
/*    int ans=0;
    int i;
    int rowmask = 0, dia1 = 0, dia2 = 0;
 
    #pragma omp parallel for reduction(+:ans) shared(i,rowmask)
    for( i = 0; i < n; i++)
    {
        rowmask = 0;
        dia1 = 0, dia2 = 0;
        int col = 0,row = i;
        ans += nqueen(1, rowmask|1<<row, dia1|(1<<(row+col)), dia2|(1<<((row+n-1)-col)));
    }*/
    printf("Case %d: %d\n", cases, ans);
 
    cases += 1;
  }
  return 0;
}
/* end */
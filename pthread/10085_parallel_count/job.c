#include "utils.h"
 
int ret[49];
//std::vector<int> x;
 
int run(int n, int key) {
    int sum = 0;
    f(n, key, ret, ret+16, ret+32, ret+48);
//    f(n, key, x, x+16, x+32, x+64);
//    std::sort(x.begin(), x.end());
//    for (int a:x)
//        sum += a;
 
    int tmp = 0;
    for (int i = 0; i <= 48; i = i + 16)
    {
        for (int j = 0; j <= 48; j = j + 16)
        {
            if(ret[i] > ret[j])
            {
                tmp = ret[j];
                ret[j] = ret[i];
                ret[j] = tmp;
            }
        }
    }
 
    for (int i = 0; i <=48; i = i + 16)
        sum += ret[i];
 
    return sum;
}
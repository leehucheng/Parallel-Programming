#define uint32_t unsigned int
static inline uint32_t rotate_left(uint32_t x, uint32_t n) {
    return  (x << n) | (x >> (32-n));
}
static inline uint32_t encrypt(uint32_t m, uint32_t key) {
    return (rotate_left(m, key&31) + key)^key;
}
 
__kernel void vecdot(uint32_t key1, uint32_t key2,  __global int* C)
{
    uint32_t idx = get_global_id(0);
    uint32_t sum = 0;
    for (uint32_t i = idx*32; i < 32*(idx+1); i++)
    {
        sum += encrypt(i, key1) * encrypt(i, key2);
    }
    C[idx] = sum;
}
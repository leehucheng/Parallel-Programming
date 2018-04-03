#define uint32_t unsigned int
static inline uint32_t rotate_left(uint32_t x, uint32_t n) {
    return  (x << n) | (x >> (32-n));
}
static inline uint32_t encrypt(uint32_t m, uint32_t key) {
    return (rotate_left(m, key&31) + key)^key;
}
 
__kernel void vecdot(uint32_t key1, uint32_t key2,  __global int* C, int CHUNK, int N)
{
    int global_idx = get_global_id(0);
    int group_idx = get_group_id(0);
    int local_idx = get_local_id(0);
 
    __local uint32_t sum_local[512];
    int sum = 0;
 
// CHUNK_SIZE & Padding distribution
    int start = global_idx * CHUNK;
    int end = (global_idx + 1) * CHUNK;
 
    if (end > N)
        end = N;
 
    for (int i = start; i < end; i++)
        sum += encrypt(i, key1) * encrypt(i, key2);
    sum_local[local_idx] = sum;
    barrier(CLK_LOCAL_MEM_FENCE);
 
// Completely Unroll
    if (local_idx<256)
    {
        sum_local[local_idx] += sum_local[local_idx + 256];
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    if (local_idx<128)
    {
        sum_local[local_idx] += sum_local[local_idx + 128];
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    if (local_idx<64)
    {
        sum_local[local_idx] += sum_local[local_idx + 64];
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    if (local_idx<32)
    {
        sum_local[local_idx] += sum_local[local_idx + 32];
        barrier(CLK_LOCAL_MEM_FENCE);
        sum_local[local_idx] += sum_local[local_idx + 16];
        barrier(CLK_LOCAL_MEM_FENCE);
        sum_local[local_idx] += sum_local[local_idx + 8];
        barrier(CLK_LOCAL_MEM_FENCE);
        sum_local[local_idx] += sum_local[local_idx + 4];
        barrier(CLK_LOCAL_MEM_FENCE);
        sum_local[local_idx] += sum_local[local_idx + 2];
        barrier(CLK_LOCAL_MEM_FENCE);
        sum_local[local_idx] += sum_local[local_idx + 1];
        if (local_idx == 0)
            C[group_idx] = sum_local[0];
    }
}
#include <stdio.h>
#include <assert.h>
#include <omp.h>
#include <inttypes.h>
//#include "utils.h"
#include <CL/cl.h>
#include <stdint.h>
 
#define MAXGPU 1
#define MAXK 1024
 
int N;
uint32_t key1, key2;
cl_int status;
cl_platform_id platform_id;
cl_uint platform_id_got;
cl_device_id GPU[MAXGPU];
cl_uint GPU_id_got;
cl_context context;
cl_command_queue commandQueue;
cl_program program ;
 
uint32_t sum;
//uint32_t C[16777216];
 
static inline uint32_t rotate_left(uint32_t x, uint32_t n) {
    return  (x << n) | (x >> (32-n));
}
static inline uint32_t encrypt(uint32_t m, uint32_t key) {
    return (rotate_left(m, key&31) + key)^key;
}
 
int main(int argc, char *argv[]) {
    /******************** Information Query - Platform ****************************/
        status = clGetPlatformIDs(1, &platform_id,
                                 &platform_id_got);
        /******************** Information Query - Device ****************************/
        status = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, MAXGPU, GPU, &GPU_id_got);
        /******************** Program Execution - Context ****************************/
        context = clCreateContext(NULL, 1, GPU, NULL, NULL, &status);
        /******************** Program Execution - Command Queue ****************************/
        commandQueue = clCreateCommandQueue(context, GPU[0], 0, &status);
        /******************** Program Execution - Program ****************************/
        FILE *kernelfp = fopen("vecdot.cl", "r"); // store kernel in kernel.cl
        assert(kernelfp != NULL);
        char kernelBuffer[MAXK]; // store into the buffer
        const char *constKernelSource = kernelBuffer; // use another pointer to buffer , const needed
        size_t kernelLength =
            fread(kernelBuffer, 1, MAXK, kernelfp); // from kernelfp file to read to kernel buffer. start from 1st character
        program = clCreateProgramWithSource(context, 1, &constKernelSource,
                                            &kernelLength, &status);
        /******************** Program Execution - Build ****************************/
        status =
        clBuildProgram(program, 1, GPU, NULL, NULL,
                        NULL);
        /******************** Program Execution - Kernel ****************************/
        cl_kernel kernel = clCreateKernel(program, "vecdot", &status);
        cl_uint* C = (cl_uint*)malloc(16777216/32 * sizeof(cl_uint));
        cl_mem bufferC =
            clCreateBuffer(context,
                CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                16777216 / 32 * sizeof(cl_uint), C, &status);
    while (scanf("%d %" PRIu32 " %" PRIu32, &N, &key1, &key2) == 3) {
        sum = 0;
        /******************** Program Execution - Vectors ****************************/
 
        /******************** Program Execution - Buffer ****************************/
 
        /******************** Program Execution - Parameter Linking ****************************/
        status = clSetKernelArg(kernel, 0, sizeof(cl_uint),
                    (void*)&key1);
 //        assert(status == CL_SUCCESS);
        status = clSetKernelArg(kernel, 1, sizeof(cl_uint),
                    (void*)&key2);
  //       assert(status == CL_SUCCESS);
        status = clSetKernelArg(kernel, 2, sizeof(cl_mem),
                    (void*)&bufferC);
   //      assert(status == CL_SUCCESS);
        /******************** Program Execution - Shape of Data ****************************/
        int CHUNK = 32;
        size_t globalThreads[] = {(size_t)N/CHUNK/32*32}; // set domain as one dim . the size is N
        size_t localThreads[] = {32}; // size of workgroup is set to 32. so there will be (N/32) work groups.each work group has 32 work item.
        status =
            clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL,
                    globalThreads, localThreads,
                    0, NULL, NULL);
   //      assert(status == CL_SUCCESS);
        /******************** Program Execution - Get Vectors ****************************/
        clEnqueueReadBuffer(commandQueue, bufferC, CL_TRUE,
              0, N/32 * sizeof(cl_uint), C,
              0, NULL, NULL);
 
        omp_set_num_threads(1);
//        #pragma omp parallel for schedule(static) reduction(+: sum1)
        for (uint32_t i = 0; i < globalThreads[0]; i++)
            sum += C[i];
 
//        printf("%" PRIu32 "\n", sum);     
        uint32_t pad = N%32;
//        printf("pad = %"PRIu32"\n", pad);
        #pragma omp parallel for schedule(static) reduction(+: sum)     
        for (uint32_t i = globalThreads[0]*CHUNK; i < N ; i++)
            sum += encrypt(i, key1) * encrypt(i, key2);
 
        printf("%" PRIu32 "\n", sum);
 
        /******************** Program Execution - Check and Free ****************************/
 //       free(C);
 
    }
        clReleaseMemObject(bufferC);
        free(C);
        clReleaseContext(context);    /* context etcmake */
        clReleaseCommandQueue(commandQueue);
        clReleaseProgram(program);
        clReleaseKernel(kernel);
    return 0;
}
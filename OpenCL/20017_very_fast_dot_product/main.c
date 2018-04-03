#include <stdio.h>
#include <assert.h>
//#include <omp.h>
#include <inttypes.h>
#include <CL/cl.h>
#include <stdint.h>
 
#define MAXGPU 1
#define MAXK 32767
#define MAXN 67108864
#define LOCAL 512
 
int N;
uint32_t key1, key2;
int CHUNK = 512;
size_t local = LOCAL;
 
cl_int status;
cl_platform_id platform_id;
cl_uint platform_id_got;
cl_device_id GPU[MAXGPU];
cl_uint GPU_id_got;
cl_context context;
cl_command_queue commandQueue;
cl_program program ;
cl_kernel kernel;
 
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
    status = clGetPlatformIDs(1, &platform_id, &platform_id_got);
    /******************** Information Query - Device ****************************/
    status = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, MAXGPU, GPU, &GPU_id_got);
    /******************** Program Execution - Context ****************************/
    context = clCreateContext(NULL, 1, GPU, NULL, NULL, &status);
    /******************** Program Execution - Command Queue ****************************/
    commandQueue = clCreateCommandQueue(context, GPU[0], CL_QUEUE_PROFILING_ENABLE, &status);
    /******************** Program Execution - Program ****************************/
    FILE *kernelfp = fopen("vecdot.cl", "r"); // store kernel in kernel.cl
    assert(kernelfp != NULL);
    char kernelBuffer[MAXK]; // store into the buffer
    const char *constKernelSource = kernelBuffer; // use another pointer to buffer , const needed
    size_t kernelLength = fread(kernelBuffer, 1, MAXK, kernelfp); // from kernelfp file to read to kernel buffer. start from 1st character
    program = clCreateProgramWithSource(context, 1, &constKernelSource, &kernelLength, &status);
    /******************** Program Execution - Build ****************************/
    status = clBuildProgram(program, 1, GPU, NULL, NULL, NULL);
    if(status != CL_SUCCESS){
        char prog_log[4096];
        size_t logLength;
        clGetProgramBuildInfo(program, GPU[0], CL_PROGRAM_BUILD_LOG, 4096, prog_log, &logLength);
        printf("%s", prog_log);
        return 0;
    }
    /******************** Program Execution - Kernel ****************************/
    kernel = clCreateKernel(program, "vecdot", &status);
    /******************** Program Execution - Vectors ****************************/
    /******************** Program Execution - Buffer ****************************/
    cl_uint* C = (cl_uint*)malloc( (((MAXN+511)/512 + LOCAL - 1)/LOCAL) * sizeof(cl_uint));
    cl_mem bufferC = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, (((MAXN+511)/512 + LOCAL - 1)/LOCAL) * sizeof(cl_uint), C, &status);
 
    while (scanf("%d %" PRIu32 " %" PRIu32, &N, &key1, &key2) == 3) {
        sum = 0;
        /******************** Program Execution - Parameter Linking ****************************/
        status = clSetKernelArg(kernel, 0, sizeof(cl_uint), (void*)&key1);
        status = clSetKernelArg(kernel, 1, sizeof(cl_uint), (void*)&key2);
        status = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&bufferC);
        status = clSetKernelArg(kernel, 3, sizeof(cl_uint), (void*)&CHUNK);
        status = clSetKernelArg(kernel, 4, sizeof(cl_uint), (void*)&N);
        /******************** Program Execution - Shape of Data ****************************/
 
        int global_num = (((N/512)/LOCAL)+1)*LOCAL;
        size_t globalThreads[] = {global_num}; // set domain as one dim . the size is N
        size_t localThreads[] = {local}; // size of workgroup is set to 512. so there will be (N/32) work groups.each work group has 32 work item.
        status = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL,
                                        globalThreads, localThreads,
                                        0, NULL, NULL);
   //      assert(status == CL_SUCCESS);
        /******************** Program Execution - Get Vectors ****************************/
        clEnqueueReadBuffer(commandQueue, bufferC, CL_TRUE,
                            0, (global_num/LOCAL) * sizeof(cl_uint), C,
                            0, NULL, NULL);
 
//        omp_set_num_threads(1);
//        #pragma omp parallel for schedule(static) reduction(+: sum1)
        for (uint32_t i = 0; i < (global_num/LOCAL); i++)
            sum += C[i];
 
//        printf("%" PRIu32 "\n", sum);    
//        #pragma omp parallel for schedule(static) reduction(+: sum)    
/*        for (uint32_t i = globalThreads[0]; i < N ; i++)
            sum += encrypt(i, key1) * encrypt(i, key2);*/
        printf("%" PRIu32 "\n", sum);
    }
    /******************** Program Execution - Check and Free ****************************/
    clReleaseMemObject(bufferC);
    free(C);
    clReleaseContext(context);    /* context etcmake */
    clReleaseCommandQueue(commandQueue);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    return 0;
}
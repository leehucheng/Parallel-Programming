#include <stdio.h>
#include <assert.h>
#include <omp.h>
#include <inttypes.h>
#include <CL/cl.h>
#include <stdint.h>
 
#define MAXGPU 10
#define MAXK 32767
#define MAXN 1073741824
#define LOCAL 512
#define DEVICENUM 2
 
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
cl_command_queue commandQueue[2];
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
    context = clCreateContext(NULL, DEVICENUM, GPU, NULL, NULL, &status);
    /******************** Program Execution - Command Queue ****************************/
    for (int device = 0; device < DEVICENUM; device++)
    {
        commandQueue[device] = clCreateCommandQueue(context, GPU[device], CL_QUEUE_PROFILING_ENABLE, &status);
    }
 
    /******************** Program Execution - Program ****************************/
    FILE *kernelfp = fopen("vecdot.cl", "r"); // store kernel in kernel.cl
    assert(kernelfp != NULL);
    char kernelBuffer[MAXK]; // store into the buffer
    const char *constKernelSource = kernelBuffer; // use another pointer to buffer , const needed
    size_t kernelLength = fread(kernelBuffer, 1, MAXK, kernelfp); // from kernelfp file to read to kernel buffer. start from 1st character
    program = clCreateProgramWithSource(context, 1, &constKernelSource, &kernelLength, &status);
    /******************** Program Execution - Build ****************************/
    status = clBuildProgram(program, DEVICENUM, GPU, NULL, NULL, NULL);
    if(status != CL_SUCCESS){
        char prog_log[4096];
        size_t logLength;
        for (int device = 0; device < DEVICENUM; device++)
        {
            clGetProgramBuildInfo(program, GPU[device], CL_PROGRAM_BUILD_LOG, 4096, prog_log, &logLength);
            printf("%s\n", prog_log);
        }
        return 0;
    }
    /******************** Program Execution - Kernel ****************************/
    kernel = clCreateKernel(program, "vecdot", &status);
    /******************** Program Execution - Vectors ****************************/
    /******************** Program Execution - Buffer ****************************/
    int itemperdevice = MAXN/LOCAL/CHUNK/DEVICENUM;
    cl_uint* C = (cl_uint*)malloc((MAXN/LOCAL/CHUNK) * sizeof(cl_uint));
 //   cl_uint* D = (cl_uint*)malloc((((MAXN+511)/512 + LOCAL - 1)/LOCAL) * sizeof(cl_uint));
    cl_mem bufferC[DEVICENUM];
    for (int device = 0; device < DEVICENUM; device++)
    {
        bufferC[device] = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, itemperdevice * sizeof(cl_uint), ((cl_uint *)C) + device * itemperdevice, &status);
    }
 
/*    bufferC[0] = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, itemperdevice * sizeof(cl_uint), (cl_uint *)C, &status);
    bufferC[1] = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, itemperdevice * sizeof(cl_uint), (cl_uint *)D, &status); */
 
    while (scanf("%d %" PRIu32 " %" PRIu32, &N, &key1, &key2) == 3) {
        sum = 0;
        int global_num = N/(LOCAL*CHUNK*DEVICENUM);
        if (N % (LOCAL*CHUNK*DEVICENUM))
            global_num++;
        size_t globalThreads[] = {(size_t)global_num*LOCAL}; // set domain as one dim . the size is N
        size_t localThreads[] = {local}; // size of workgroup is set to 512. so there will be (N/32) work groups.each work group has 32 work item.
        cl_event events[DEVICENUM];
        /******************** Program Execution - Parameter Linking ****************************/
        /*for (int device =0; device < DEVICENUM; device++)
        {
            status = clSetKernelArg(kernel, 0, sizeof(cl_uint), (void*)&key1);
            status = clSetKernelArg(kernel, 1, sizeof(cl_uint), (void*)&key2);
            status = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&bufferC[device]);
            status = clSetKernelArg(kernel, 3, sizeof(cl_uint), (void*)&CHUNK);
            status = clSetKernelArg(kernel, 4, sizeof(cl_uint), (void*)&N);
 
            status = clEnqueueNDRangeKernel(commandQueue[device], kernel, 1, NULL,
                                            globalThreads, localThreads,
                                            0, NULL, &(events[device]));
        }*/
 
        int offset = 0;
        int num = global_num*LOCAL*CHUNK;
 
        status = clSetKernelArg(kernel, 0, sizeof(cl_uint), (void*)&key1);
        status = clSetKernelArg(kernel, 1, sizeof(cl_uint), (void*)&key2);
        status = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&bufferC[0]);
        status = clSetKernelArg(kernel, 3, sizeof(cl_uint), (void*)&CHUNK);
        status = clSetKernelArg(kernel, 4, sizeof(cl_uint), (void*)&num);
        status = clSetKernelArg(kernel, 5, sizeof(cl_uint), (void*)&offset);
 
        status = clEnqueueNDRangeKernel(commandQueue[0], kernel, 1, NULL,
                                        globalThreads, localThreads,
                                        0, NULL, &(events[0]));
 
        offset = global_num*LOCAL;
 
        status = clSetKernelArg(kernel, 0, sizeof(cl_uint), (void*)&key1);
        status = clSetKernelArg(kernel, 1, sizeof(cl_uint), (void*)&key2);
        status = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&bufferC[1]);
        status = clSetKernelArg(kernel, 3, sizeof(cl_uint), (void*)&CHUNK);
        status = clSetKernelArg(kernel, 4, sizeof(cl_uint), (void*)&N);
        status = clSetKernelArg(kernel, 5, sizeof(cl_uint), (void*)&offset);
 
        status = clEnqueueNDRangeKernel(commandQueue[1], kernel, 1, NULL,
                                        globalThreads, localThreads,
                                        0, NULL, &(events[1]));
 
 //       clFinish(commandQueue[0]);
 //       clFinish(commandQueue[1]);
        /******************** Program Execution - Shape of Data ****************************/
        //clWaitForEvents(DEVICENUM, events);
        //clFinish(commandQueue[0]);
        //clFinish(commandQueue[1]);
//        assert(status == CL_SUCCESS);
        /******************** Program Execution - Get Vectors ****************************/
 
        for (int device = 0; device < DEVICENUM; device++)
        {
            clEnqueueReadBuffer(commandQueue[device], bufferC[device], CL_TRUE,
                                0, itemperdevice * sizeof(cl_uint), ((cl_uint*)C) + device * itemperdevice,
                                0, NULL, NULL);
        }
 
 //       omp_set_num_threads(4);
        for(int device = 0; device < DEVICENUM; device++)
        {
 //           #pragma omp parallel for reduction(+: sum)
            for (uint32_t i = 0; i < global_num ; i++)
                sum += C[i + device * itemperdevice];
        }
 
 /*       #pragma omp parallel for reduction(+: sum)
        for (uint32_t i = itemperdevice ; i < (global_num/LOCAL)/DEVICENUM + itemperdevice; i++)
            sum += C[i];*/
 
//        printf("%" PRIu32 "\n", sum);    
//        #pragma omp parallel for schedule(static) reduction(+: sum)    
/*        for (uint32_t i = globalThreads[0]; i < N ; i++)
            sum += encrypt(i, key1) * encrypt(i, key2);*/
        printf("%" PRIu32 "\n", sum);
 
    }
    /******************** Program Execution - Check and Free ****************************/
    clReleaseMemObject(bufferC[0]);
    clReleaseMemObject(bufferC[1]);
    free(C);
//    free(D);
    clReleaseContext(context);    /* context etcmake */
    clReleaseCommandQueue(commandQueue[0]);
    clReleaseCommandQueue(commandQueue[1]);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    return 0;
}
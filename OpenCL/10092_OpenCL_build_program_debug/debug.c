/* header */
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
 
#include <stdio.h>
#include <assert.h>
#include <CL/cl.h>
 
#define MAXGPU 10
#define MAXK 1024
#define N (1024 * 1024)
/* main */
cl_int status;
cl_platform_id platform_id;
cl_uint platform_id_got;
cl_device_id GPU[MAXGPU];
cl_uint GPU_id_got;
cl_context context;
cl_command_queue commandQueue;
cl_program program;
 
int main(int argc, char *argv[])
{
  /******************** Information Query - Platform ****************************/
  status = clGetPlatformIDs(1, &platform_id,
                &platform_id_got);
  assert(status == CL_SUCCESS && platform_id_got == 1);
//  printf("%d platform found\n", platform_id_got);
  /******************** Information Query - Device ****************************/
  status = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU,
              MAXGPU, GPU, &GPU_id_got);
  assert(status == CL_SUCCESS);
//  printf("There are %d GPU devices\n", GPU_id_got);
  /******************** Program Execution - Context ****************************/
  context = clCreateContext(NULL, GPU_id_got, GPU, NULL, NULL,
            &status);
  assert(status == CL_SUCCESS);
  /******************** Program Execution - Command Queue ****************************/
  commandQueue = clCreateCommandQueue(context, GPU[0], 0, &status);
  assert(status == CL_SUCCESS);
  /******************** Program Execution - Program ****************************/
  char input[30];
  assert(scanf("%s", input) == 1);
  FILE *kernelfp = fopen(input, "r");
  assert(kernelfp != NULL);
  char kernelBuffer[MAXK]; // store into the buffer
  const char *constKernelSource = kernelBuffer; // use another pointer to buffer , const needed
  size_t kernelLength =
    fread(kernelBuffer, 1, MAXK, kernelfp); // from kernelfp file to read to kernel buffer. start from 1st character
//  printf("The size of kernel source is %zu\n", kernelLength);
  program = clCreateProgramWithSource(context, 1, &constKernelSource,
                  &kernelLength, &status);
  assert(status == CL_SUCCESS);
  /******************** Program Execution - Build ****************************/
  status =
    clBuildProgram(program, GPU_id_got, GPU, NULL, NULL,
           NULL);
  if (status != CL_SUCCESS){
        char program_log[100000];
        clGetProgramBuildInfo(program,
                               GPU[0],
                               CL_PROGRAM_BUILD_LOG,
                               sizeof(program_log),
                               program_log,
                               NULL);
        printf("%s", program_log);
        return 0;
  }  
//  assert(status == CL_SUCCESS);
//  printf("Build program completes\n");
  /******************** Program Execution - Kernel ****************************/
  cl_kernel kernel = clCreateKernel(program, "add", &status);
  assert(status == CL_SUCCESS);
//  printf("Build kernel completes\n");
  /******************** Program Execution - Vectors ****************************/
  cl_uint* A = (cl_uint*)malloc(N * sizeof(cl_uint));
  cl_uint* B = (cl_uint*)malloc(N * sizeof(cl_uint));
  cl_uint* C = (cl_uint*)malloc(N * sizeof(cl_uint));
  assert(A != NULL && B != NULL && C != NULL);
 
  for (int i = 0; i < N; i++) {
    A[i] = i;
    B[i] = N - i;
  }
  /******************** Program Execution - Buffer ****************************/
  cl_mem bufferA =
    clCreateBuffer(context,
           CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
           N * sizeof(cl_uint), A, &status);
  assert(status == CL_SUCCESS);
  cl_mem bufferB =
    clCreateBuffer(context,
           CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
           N * sizeof(cl_uint), B, &status);
  assert(status == CL_SUCCESS);
  cl_mem bufferC =
    clCreateBuffer(context,
           CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
           N * sizeof(cl_uint), C, &status);
  assert(status == CL_SUCCESS);
//  printf("Build buffers completes\n");
  /******************** Program Execution - Parameter Linking ****************************/
  status = clSetKernelArg(kernel, 0, sizeof(cl_mem),
              (void*)&bufferA);
  assert(status == CL_SUCCESS);
  status = clSetKernelArg(kernel, 1, sizeof(cl_mem),
              (void*)&bufferB);
  assert(status == CL_SUCCESS);
  status = clSetKernelArg(kernel, 2, sizeof(cl_mem),
              (void*)&bufferC);
  assert(status == CL_SUCCESS);
//  printf("Set kernel arguments completes\n");
  /******************** Program Execution - Shape of Data ****************************/
  size_t globalThreads[] = {(size_t)N}; // set domain as one dim . the size is N
  size_t localThreads[] = {1}; // size of workgroup is set to 1. so there will be N work groups.each work group has one work item.
  status =
    clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL,
               globalThreads, localThreads,
               0, NULL, NULL);
  assert(status == CL_SUCCESS);
//  printf("Specify the shape of the domain completes.\n");
  /******************** Program Execution - Get Vectors ****************************/
  clEnqueueReadBuffer(commandQueue, bufferC, CL_TRUE,
              0, N * sizeof(cl_uint), C,
              0, NULL, NULL);
//  printf("Kernel execution completes.\n");
  /******************** Program Execution - Check and Free ****************************/
 
  for (int i = 0; i < N; i++)
    assert(A[i] + B[i] == C[i]);
 
  free(A);            /* host memory */
  free(B);
  free(C);
  clReleaseContext(context);    /* context etcmake */
  clReleaseCommandQueue(commandQueue);
  clReleaseProgram(program);
  clReleaseKernel(kernel);
  clReleaseMemObject(bufferA);    /* buffers */
  clReleaseMemObject(bufferB);
  clReleaseMemObject(bufferC);
  return 0;
}
/* end */
/*
Build: g++ cltest2.cpp -o cltest2 -lOpenCL
*/

#include <CL/cl.h>
#include <CL/cl.hpp>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <fstream>
#include <sstream>

#include "kernelcode.hpp"

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#define VECTOR_SIZE3 50000

using namespace std;
using namespace std::chrono;

// OpenCL kernel which is run for every work item created.

//Az adatbazis tablajaval megegyzo struktura letrehozasa.
typedef struct table3 {
  int c1p3;
  int c2;
  int c3;
  int c4;
  int fk_p1_p3;
  int fk_p2_p3;
} Table3Type;

int main(void) {


    // Get platform and device information
  cl_platform_id *platforms = NULL;
  cl_uint num_platforms;
  // Set up the Platform
  cl_int clStatus = clGetPlatformIDs(0, NULL, &num_platforms);
  platforms = (cl_platform_id *)malloc(sizeof(cl_platform_id) * num_platforms);
  clStatus = clGetPlatformIDs(num_platforms, platforms, NULL);

  // Get the devices list and choose the device you want to run on
  cl_device_id *device_list = NULL;
  cl_uint num_devices;
  size_t value_size;
  char *value;

  clStatus =
      clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);
  device_list = (cl_device_id *)malloc(sizeof(cl_device_id) * num_devices);
  clStatus = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, num_devices,
                            device_list, NULL);

  clGetDeviceInfo(device_list[0], CL_DEVICE_NAME, 0, NULL, &value_size);
  value = (char*) malloc(value_size);
  clGetDeviceInfo(device_list[0], CL_DEVICE_NAME, value_size, value, &value_size);
  printf("%d. Device: %s\n", 0, value);
            free(value);
                            

  // Create one OpenCL context for each device in the platform
  cl_context context;
  context =
      clCreateContext(NULL, num_devices, device_list, NULL, NULL, &clStatus);

  // Create a command queue
  cl_command_queue command_queue =
      clCreateCommandQueueWithProperties(context, device_list[0], 0, &clStatus);

    //---SPEED TEST START---
  auto start = high_resolution_clock::now();
  //----------------------

//Tabla, kimeneti tabla, visszatero sorok szama, szemafor valtozo
  Table3Type *t3 = (Table3Type *)malloc(sizeof(Table3Type) * VECTOR_SIZE3);
  Table3Type *tout = (Table3Type *)malloc(sizeof(Table3Type) * VECTOR_SIZE3);
  int *returned_rows = (int *)malloc(sizeof(int));
  int *mutex = (int *)malloc(sizeof(int));
  *mutex=0;

// Tabla feltoltese a fajlbol
  string line;
  ifstream is;
  int count = 0;

  is.open("t3.txt");
  count = 0;
  while (getline(is, line)) {
    std::istringstream iss(line);
    if (!(iss >> t3[count].c1p3 >> t3[count].c2 >> t3[count].c3 >>
          t3[count].c4 >> t3[count].fk_p1_p3 >> t3[count].fk_p2_p3)) {
      break;
    }
    count++;
  }

  ////////////
  // Create memory buffers on the device for each vector
  cl_mem Table3_clmem =
      clCreateBuffer(context, CL_MEM_READ_ONLY,
                     VECTOR_SIZE3 * sizeof(Table3Type), NULL, &clStatus);
  cl_mem TableOUT_clmem =
      clCreateBuffer(context, CL_MEM_READ_WRITE,
                     VECTOR_SIZE3 * sizeof(Table3Type), NULL, &clStatus);
  cl_mem Returned_Rows_clmem =
      clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &clStatus);
  cl_mem Mutex_clmem =
      clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &clStatus);

  // Copy the Buffer A and B to the device
  clStatus = clEnqueueWriteBuffer(command_queue, Table3_clmem, CL_TRUE, 0,
                                  VECTOR_SIZE3 * sizeof(Table3Type), t3, 0,
                                  NULL, NULL);

  clStatus = clEnqueueWriteBuffer(command_queue, Mutex_clmem, CL_TRUE, 0,
                                  sizeof(int), mutex, 0,
                                  NULL, NULL);
  ////////////

  // Create a program from the kernel source
  cl_program program = clCreateProgramWithSource(
      context, 1, (const char **)&t3tovramKernel, NULL, &clStatus);

  // Build the program
  clStatus = clBuildProgram(program, 1, device_list, NULL, NULL, NULL);

  // Create the OpenCL kernel
  cl_kernel kernel = clCreateKernel(program, "t3tovramKernel", &clStatus);

  ///////////
  // Set the arguments of the kernel
  clStatus =
      clSetKernelArg(kernel, 0, sizeof(cl_mem), (Table3Type *)&Table3_clmem);
  clStatus =
      clSetKernelArg(kernel, 1, sizeof(cl_mem), (Table3Type *)&TableOUT_clmem);
  clStatus =
      clSetKernelArg(kernel, 2, sizeof(cl_mem), (int *)&Returned_Rows_clmem);

  clStatus = clSetKernelArg(kernel, 3, sizeof(cl_mem), (int *)&Mutex_clmem);

  // Execute the OpenCL kernel on the list
  size_t global_size = VECTOR_SIZE3; // Process the entire lists
  size_t local_size = 100;            // Process one item at a time
  clStatus = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
                                    &global_size, &local_size, 0, NULL, NULL);

  // Read the cl memory C_clmem on device to the host variable C
  clStatus = clEnqueueReadBuffer(command_queue, TableOUT_clmem, CL_TRUE, 0,
                                 VECTOR_SIZE3 * sizeof(Table3Type), tout, 0,
                                 NULL, NULL);
  clStatus = clEnqueueReadBuffer(command_queue, Returned_Rows_clmem, CL_TRUE, 0,
                                 sizeof(int), returned_rows, 0, NULL, NULL);

  // Clean up and wait for all the comands to complete.
  clStatus = clFlush(command_queue);
  clStatus = clFinish(command_queue);

  int i;
  // Display the result to the screen

  for (i = 0; i < *returned_rows; i++)
    printf("%d \t %d \t %d \t %d \t %d \t %d \n", tout[i].c1p3, tout[i].c2,
           tout[i].c3, tout[i].c4, tout[i].fk_p1_p3, tout[i].fk_p2_p3);

  //---SPEED TEST STOP---
  auto stop = high_resolution_clock::now();
  //----------------------

  cout << *returned_rows << "\nDone.\n";

    auto duration = duration_cast<microseconds>(stop - start);

  cout << "Time taken by function: " << duration.count() << " microseconds"
       << endl;

  return 0;
}

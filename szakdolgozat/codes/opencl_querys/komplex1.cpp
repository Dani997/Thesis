/*
Build: g++ cltest2.cpp -o cltest2 -lOpenCL
g++ -D_GLIBCXX_USE_CXX11_ABI=0 komplex1.cpp -o komplex1.out -lOpenCL
-lmysqlcppconn
*/

#include <CL/cl.h>
#include <CL/cl.hpp>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <fstream>
#include <sstream>

#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#define VECTOR_SIZE3 50000
#define VECTOR_SIZE2 1000
#define VECTOR_SIZE1 100
#define THREADS 500

using namespace std;
using namespace std::chrono;

// OpenCL kernel which is run for every work item created.

typedef struct {
  int c1p3;
  int c2;
  int c3;
  int c4;
  int fk_p1_p3;
  int fk_p2_p3;
} Table3Type;
typedef struct {
  int c1p2;
  int c2;
  int c3;
  int fk_p1_p2;
} Table2Type;
typedef struct {
  int c1p1;
  int c2;
  int c3;
  int c4;
} Table1Type;

void load_database(Table3Type *T3, Table2Type *T2);
int main(void) {

  //---SPEED TEST START---
  auto start = high_resolution_clock::now();

  FILE *fp;
  fp = fopen("komplex1kernel.cl", "r");
  if (!fp) {
    fprintf(stderr, "Error loading kernel.\n");
    exit(1);
  }
  fseek(fp, 0, SEEK_END);
  size_t kernel_sz = ftell(fp);
  rewind(fp);
  char *kernel_str = (char *)malloc(kernel_sz);
  fread(kernel_str, 1, kernel_sz, fp);
  fclose(fp);

  //---SPEED TEST STOP---
  auto stop = high_resolution_clock::now();
  auto duration = duration_cast<microseconds>(stop - start);
  cout << "Kernel kod beolvasas: " << duration.count() << " microseconds"
       << endl;

  //---SPEED TEST START---
  start = high_resolution_clock::now();

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

  clStatus =
      clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);

  device_list = (cl_device_id *)malloc(sizeof(cl_device_id) * num_devices);

  clStatus = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, num_devices,
                            device_list, NULL);

  // Create one OpenCL context for each device in the platform
  cl_context context;
  context =
      clCreateContext(NULL, num_devices, device_list, NULL, NULL, &clStatus);
  // Create a command queue
  cl_command_queue command_queue =
      clCreateCommandQueueWithProperties(context, device_list[0], 0, &clStatus);

  //---SPEED TEST STOP---
  stop = high_resolution_clock::now();
  duration = duration_cast<microseconds>(stop - start);
  cout << "eszkoz, platform, kontextus, parancssor: " << duration.count()
       << " microseconds" << endl;

  //---SPEED TEST START---
  start = high_resolution_clock::now();

  Table3Type *t3 = (Table3Type *)malloc(sizeof(Table3Type) * VECTOR_SIZE3);
  Table2Type *t2 = (Table2Type *)malloc(sizeof(Table2Type) * VECTOR_SIZE2);
  Table2Type *t1 = (Table1Type *)malloc(sizeof(Table1Type) * VECTOR_SIZE1);
  Table3Type *tout = (Table3Type *)malloc(sizeof(Table3Type) * VECTOR_SIZE3);
  int *returned_rows = (int *)malloc(sizeof(int) * THREADS);

  //---SPEED TEST STOP---
  stop = high_resolution_clock::now();
  duration = duration_cast<microseconds>(stop - start);
  cout << "C valtozok letrehozasa: " << duration.count() << " microseconds"
       << endl;
  //---SPEED TEST START---
  start = high_resolution_clock::now();

  load_database(t3, t2);

  //---SPEED TEST STOP---
  stop = high_resolution_clock::now();
  duration = duration_cast<microseconds>(stop - start);
  cout << "T2, T3 tablak lekerdezese " << duration.count() << " microseconds"
       << endl;

  ////////////
  // Create memory buffers on the device for each vector
  cl_mem Table3_clmem =
      clCreateBuffer(context, CL_MEM_READ_ONLY,
                     VECTOR_SIZE3 * sizeof(Table3Type), NULL, &clStatus);

  cl_mem Table2_clmem =
      clCreateBuffer(context, CL_MEM_READ_ONLY,
                     VECTOR_SIZE2 * sizeof(Table2Type), NULL, &clStatus);

  cl_mem TableOUT_clmem =
      clCreateBuffer(context, CL_MEM_READ_WRITE,
                     VECTOR_SIZE3 * sizeof(Table3Type), NULL, &clStatus);

  cl_mem Returned_Rows_clmem = clCreateBuffer(
      context, CL_MEM_READ_WRITE, THREADS * sizeof(int), NULL, &clStatus);

  //---SPEED TEST START---
  start = high_resolution_clock::now();

  // Copy the Buffer to the device
  clStatus = clEnqueueWriteBuffer(command_queue, Table3_clmem, CL_TRUE, 0,
                                  VECTOR_SIZE3 * sizeof(Table3Type), t3, 0,
                                  NULL, NULL);

  //---SPEED TEST STOP---
  stop = high_resolution_clock::now();
  duration = duration_cast<microseconds>(stop - start);
  cout << "CL memoria bufferek letrehozasa es feltoltese " << duration.count()
       << " microseconds" << endl;


  clStatus = clEnqueueWriteBuffer(command_queue, Table2_clmem, CL_TRUE, 0,
                                  VECTOR_SIZE2 * sizeof(Table2Type), t2, 0,
                                  NULL, NULL);

  //---SPEED TEST START---
  start = high_resolution_clock::now();

  ////////////

  // Create a program from the kernel source
  cl_program program =
      clCreateProgramWithSource(context, 1, (const char **)&kernel_str,
                                (const size_t *)&kernel_sz, &clStatus);

  //---SPEED TEST STOP---
  stop = high_resolution_clock::now();
  duration = duration_cast<microseconds>(stop - start);
  cout << "Kernel kod letrehozasa forrasbol  " << duration.count()
       << " microseconds" << endl;
  //---SPEED TEST START---
  start = high_resolution_clock::now();

  // Build the program
  clStatus = clBuildProgram(program, 1, device_list, NULL, NULL, NULL);

  //---SPEED TEST STOP---
  stop = high_resolution_clock::now();
  duration = duration_cast<microseconds>(stop - start);
  cout << "Kernel kod felepitese " << duration.count() << " microseconds"
       << endl;
  //---SPEED TEST START---
  start = high_resolution_clock::now();

  // Create the OpenCL kernel
  cl_kernel kernel = clCreateKernel(program, "komplex1Kernel", &clStatus);

  //---SPEED TEST STOP---
  stop = high_resolution_clock::now();
  duration = duration_cast<microseconds>(stop - start);
  cout << "Opencl kernel letrehozasa " << duration.count() << " microseconds"
       << endl;
  //---SPEED TEST START---
  start = high_resolution_clock::now();

  ///////////
  // Set the arguments of the kernel
  clStatus =
      clSetKernelArg(kernel, 0, sizeof(cl_mem), (Table3Type *)&Table3_clmem);
  clStatus =
      clSetKernelArg(kernel, 1, sizeof(cl_mem), (Table2Type *)&Table2_clmem);
  clStatus =
      clSetKernelArg(kernel, 2, sizeof(cl_mem), (Table3Type *)&TableOUT_clmem);
  clStatus =
      clSetKernelArg(kernel, 3, sizeof(cl_mem), (int *)&Returned_Rows_clmem);

  // Execute the OpenCL kernel on the list
  size_t global_size = 500; // Process the entire lists
  size_t local_size = 10;   // Process one item at a time

  clStatus = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
                                    &global_size, &local_size, 0, NULL, NULL);

  // Read the cl memory C_clmem on device to the host variable C
  clStatus = clEnqueueReadBuffer(command_queue, TableOUT_clmem, CL_TRUE, 0,
                                 VECTOR_SIZE3 * sizeof(Table3Type), tout, 0,
                                 NULL, NULL);
  clStatus =
      clEnqueueReadBuffer(command_queue, Returned_Rows_clmem, CL_TRUE, 0,
                          THREADS * sizeof(int), returned_rows, 0, NULL, NULL);

  // Clean up and wait for all the comands to complete.
  clStatus = clFlush(command_queue);
  clStatus = clFinish(command_queue);

  //---SPEED TEST STOP---
  stop = high_resolution_clock::now();
  duration = duration_cast<microseconds>(stop - start);
  cout << "Argumentum atadas, futtatas, buffekerek "
          "kiolvasasa, varakozas a befejezesre "
       << duration.count() << " microseconds" << endl;
  //---SPEED TEST START---
  start = high_resolution_clock::now();

  int i;
  // Display the result to the screen
  ofstream myfile;
  myfile.open("returndb.txt");

  for (i = 0; i < 500; i++) {
    for (int j = 0; j < returned_rows[i]; j++) {
      int van = i + j * 100;
      myfile << van << "| \t" << tout[van].c1p3 << "\t" << tout[van].c2 << "\t"
             << tout[van].c3 << "\t" << tout[van].c4 << "\t"
             << tout[van].fk_p1_p3 << "\t" << tout[van].fk_p2_p3 << endl;

      /*printf("%d| \t %d \t %d \t %d \t %d \t %d \t %d \n", van,
         tout[van].c1p3, tout[van].c2, tout[van].c3, tout[van].c4,
         tout[van].fk_p1_p3, tout[van].fk_p2_p3);*/
    }
  }
  int sum = 0;
  for (i = 0; i < 500; i++) {
    sum += returned_rows[i];
  }

  /*clStatus = clReleaseKernel(kernel);
  clStatus = clReleaseProgram(program);
  clStatus = clReleaseContext(context);
  clStatus = clReleaseCommandQueue(command_queue);
  free(platforms);
  free(device_list);*/

  clStatus = clReleaseMemObject(Table3_clmem);
  clStatus = clReleaseMemObject(Table2_clmem);
  clStatus = clReleaseMemObject(TableOUT_clmem);
  clStatus = clReleaseMemObject(Returned_Rows_clmem);
  free(t3);
  free(t2);
  free(tout);
  free(returned_rows);

  cout << sum << "\nDone.\n";

  return 0;
}

void load_database(Table3Type *T3, Table2Type *T2) {
  try {
    sql::Driver *driver;
    sql::Connection *con;
    sql::Statement *stmt;
    sql::ResultSet *res;
    sql::PreparedStatement *pstmt;

    /* Create a connection */
    driver = get_driver_instance();
    con = driver->connect("tcp://192.168.0.43:3306", "program", "a");
    /* Connect to the MySQL test database */
    con->setSchema("thesis");

    pstmt = con->prepareStatement("SELECT * FROM T3");
    res = pstmt->executeQuery();
    // res->afterLast();

    int i = 0;

    while (res->next()) {
      T3[i].c1p3 = res->getInt("c1p3");
      T3[i].c2 = res->getInt("c2");
      T3[i].c3 = res->getInt("c3");
      T3[i].c4 = res->getInt("c4");
      T3[i].fk_p1_p3 = res->getInt("fk_p1_p3");
      T3[i].fk_p2_p3 = res->getInt("fk_p2_p3");
      i++;
    }
    delete res;

    pstmt = con->prepareStatement("SELECT * FROM T2");
    res = pstmt->executeQuery();
    // res->afterLast();
    i = 0;
    while (res->next()) {
      T2[i].c1p2 = res->getInt("c1p2");
      T2[i].c2 = res->getInt("c2");
      T2[i].c3 = res->getInt("c3");
      T2[i].fk_p1_p2 = res->getInt("fk_p1_p2");

      i++;
    }
    delete res;

  } catch (sql::SQLException &e) {

    cout << "# ERR: SQLException in " << __FILE__;
    cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
    cout << "# ERR: " << e.what();
    cout << " (MySQL error code: " << e.getErrorCode();
    cout << ", SQLState: " << e.getSQLState() << " )" << endl;

    cout << "\n" << EXIT_FAILURE;
  }

  // cout << "Full query completed.\n" << endl;
}

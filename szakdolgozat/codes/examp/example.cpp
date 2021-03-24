/*
Build: g++ cltest2.cpp -o cltest2 -lOpenCL
g++ -D_GLIBCXX_USE_CXX11_ABI=0 example.cpp -o example.out -lOpenCL
-lmysqlcppconn */

#include "head.hpp"

#define PROGRAM_FILE "examp.cl"
#define VECTOR_SIZE1 100
#define GPU 1
#define CPU 0

int main(void) {
  size_t kernel_length;
  string kernel_string;
  read_kernel_code(PROGRAM_FILE, &kernel_length, &kernel_string);

  size_t local_size = 10; 
  size_t global_size = ceil( (float)VECTOR_SIZE1/local_size)  ; 
  global_size = (ceil( (float)global_size/local_size )) * local_size;

  cl_int clStatus;
	cl_device_id device;
	cl_context context;
	cl_program program;
	cl_kernel kernel;
	cl_command_queue command_queue;

  device =  create_device( GPU );

  // Create one OpenCL context for each device in the platform
context = clCreateContext(NULL, 1, &device, NULL, NULL, &clStatus);

  // Create a command queue
command_queue = clCreateCommandQueue(context, device, 0, &clStatus);


 Table1Type *t1 = (Table1Type *)malloc(sizeof(Table1Type) * VECTOR_SIZE1);
 Table1Type *tout = (Table1Type *)malloc(sizeof(Table1Type) * VECTOR_SIZE1);
 int *returned_rows = (int *)malloc(sizeof(int) * global_size );
 int *limit = (int *) malloc(sizeof(int));
  int *range = (int *) malloc(sizeof(int));
 *limit = VECTOR_SIZE1;
*range = local_size;


 load_database(t1);

  cl_mem Table1_clmem =
      clCreateBuffer(context, CL_MEM_READ_WRITE,
                     VECTOR_SIZE1 * sizeof(Table1Type), NULL, &clStatus);

  cl_mem TableOUT_clmem =
      clCreateBuffer(context, CL_MEM_READ_WRITE,
                     VECTOR_SIZE1 * sizeof(Table1Type), NULL, &clStatus);       

  cl_mem Returned_Rows_clmem = clCreateBuffer(
      context, CL_MEM_READ_WRITE, global_size * sizeof(int), NULL, &clStatus);

  cl_mem Limit_clmem = clCreateBuffer(
      context, CL_MEM_READ_WRITE, sizeof(int), NULL, &clStatus);    

  cl_mem Range_clmem = clCreateBuffer(
      context, CL_MEM_READ_WRITE, sizeof(int), NULL, &clStatus); 


    clStatus = clEnqueueWriteBuffer(command_queue, Table1_clmem, CL_TRUE, 0,
                                  VECTOR_SIZE1 * sizeof(Table1Type), t1, 0,
                                  NULL, NULL);
    clStatus = clEnqueueWriteBuffer(command_queue, Limit_clmem, CL_TRUE, 0,
                                  sizeof(int), limit, 0,
                                  NULL, NULL);                                  

    clStatus = clEnqueueWriteBuffer(command_queue, Range_clmem, CL_TRUE, 0,
                                  sizeof(int), range, 0,
                                  NULL, NULL);

  // Create a program from the kernel source
program =
      clCreateProgramWithSource(context, 1, (const char **)&kernel_string,
                                (const size_t *)&kernel_length, &clStatus);

  // Build the program
  clStatus = clBuildProgram(program, 1, &device, NULL, NULL, NULL);

  // Create the OpenCL kernel
   kernel = clCreateKernel(program, "examp", &clStatus);

  clStatus =
      clSetKernelArg(kernel, 0, sizeof(cl_mem), (Table1Type *)&Table1_clmem);
  clStatus =
      clSetKernelArg(kernel, 1, sizeof(cl_mem), (Table1Type *)&TableOUT_clmem);
  clStatus =
      clSetKernelArg(kernel, 2, sizeof(cl_mem), (int *)&Returned_Rows_clmem);
  clStatus =
      clSetKernelArg(kernel, 3, sizeof(cl_mem), (int *)&Limit_clmem);
  clStatus =
      clSetKernelArg(kernel, 4, sizeof(cl_mem), (int *)&Range_clmem);

  clStatus = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
                                    &global_size, &local_size, 0, NULL, NULL);

  clStatus = clEnqueueReadBuffer(command_queue, TableOUT_clmem, CL_TRUE, 0,
                                 VECTOR_SIZE1 * sizeof(Table1Type), tout, 0,
                                 NULL, NULL);
  clStatus =
      clEnqueueReadBuffer(command_queue, Returned_Rows_clmem, CL_TRUE, 0,
                          global_size * sizeof(int), returned_rows, 0, NULL, NULL);


  clStatus = clFlush(command_queue);
  clStatus = clFinish(command_queue);

  int i,j;
  for(i=0; i < global_size; i++)
    for(j=0; j<returned_rows[i]; j++)
    {
      {
        printf("csoport:  %d index: %d  \n",i, tout[ global_size * i + j ].c1p1  );
      }
    }

  return 0;


  clStatus = clReleaseKernel(kernel);
  clStatus = clReleaseProgram(program);
  clStatus = clReleaseContext(context);
  clStatus = clReleaseCommandQueue(command_queue);
  clStatus = clReleaseDevice(device);


  clStatus = clReleaseMemObject(Table1_clmem);
  clStatus = clReleaseMemObject(TableOUT_clmem);
  clStatus = clReleaseMemObject(Returned_Rows_clmem);
  clStatus = clReleaseMemObject(Range_clmem);
  clStatus = clReleaseMemObject(Limit_clmem);
  free(t1);
  free(tout);
  free(returned_rows);
  free(limit);
  free(range);
  
}

void read_kernel_code(char const* file, size_t* kernel_length,
                      string* kernel_string) {
  FILE* fp;
  fp = fopen(file, "r");
  if (!fp) {
    fprintf(stderr, "Error loading kernel.\n");
    exit(1);
  }
  fseek(fp, 0, SEEK_END);
  *kernel_length = ftell(fp);
  rewind(fp);
  char* kernel_str = (char*)malloc(*kernel_length);
  fread(kernel_str, 1, *kernel_length, fp);
  fclose(fp);
  *kernel_string = kernel_str;
}



void load_database(Table1Type *T1) {
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


    pstmt = con->prepareStatement("SELECT * FROM T1");
    res = pstmt->executeQuery();
    int i = 0;
    while (res->next()) {
      T1[i].c1p1 = res->getInt("c1p1");
      T1[i].c2 = res->getInt("c2");
      T1[i].c3 = res->getInt("c3");
      T1[i].c4= res->getInt("c4");
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

cl_device_id create_device(const int gpu) {

   cl_platform_id platform;
   cl_device_id dev;
   int err;

   /* Identify a platform */
   err = clGetPlatformIDs(1, &platform, NULL);
   if(err < 0) {
      perror("Couldn't identify a platform");
      exit(1);
   } 

   // Access a device
   if (gpu==1) {
      // GPU
      err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
      if(err == CL_DEVICE_NOT_FOUND) {
         // CPU
         err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
      }
   } else {
         err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
   }

   if(err < 0) {
      perror("Couldn't access any devices");
      exit(1);   
   }

   return dev;
}
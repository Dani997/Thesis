/*
Build: g++ cltest2.cpp -o cltest2 -lOpenCL
g++ -D_GLIBCXX_USE_CXX11_ABI=0 example.cpp -o example.out -lOpenCL
-lmysqlcppconn */

#include "head.hpp"

#define PROGRAM_FILE "examp.cl"
#define VECTOR_SIZE1 100


void opencl_things(  cl_int *clStatus,
  cl_platform_id *platforms,
  cl_uint *num_platforms,
  cl_device_id *device_list,
  cl_uint *num_devices,
  cl_context *context,
  cl_command_queue *command_queue);

int main(void) {
  size_t kernel_length;
  string kernel_string;
  read_kernel_code(PROGRAM_FILE, &kernel_length, &kernel_string);

  size_t local_size = 10; 
  size_t global_size = ceil( (float)VECTOR_SIZE1/local_size)  ; 
  global_size = (ceil( (float)global_size/local_size )) * local_size;



  /*cl_int clStatus;
  cl_platform_id *platforms = NULL;
  cl_uint num_platforms;
  cl_device_id *device_list = NULL;
  cl_uint num_devices;
  cl_context context;
  cl_command_queue command_queue;

 opencl_things(&clStatus, platforms, &num_platforms, device_list, &num_devices, &context, &command_queue);*/

 // Információk lekérése a paltformról és eszközről
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
  cl_program program =
      clCreateProgramWithSource(context, 1, (const char **)&kernel_string,
                                (const size_t *)&kernel_length, &clStatus);

  // Build the program
  clStatus = clBuildProgram(program, 1, device_list, NULL, NULL, NULL);

  // Create the OpenCL kernel
  cl_kernel kernel = clCreateKernel(program, "examp", &clStatus);

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

void opencl_things(  cl_int *clStatus,
  cl_platform_id *platforms,
  cl_uint *num_platforms,
  cl_device_id *device_list,
  cl_uint *num_devices,
  cl_context *context,
  cl_command_queue *command_queue)
{


    // Set up the Platform
  *clStatus = clGetPlatformIDs(0, NULL, num_platforms);
  platforms = (cl_platform_id *)malloc(sizeof(cl_platform_id) * *num_platforms);
  *clStatus = clGetPlatformIDs(*num_platforms, platforms, NULL);

  // Get the devices list and choose the device you want to run on

  /**clStatus =
      clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 0, NULL, num_devices);

  device_list = (cl_device_id *)malloc(sizeof(cl_device_id) * *num_devices);

  *clStatus = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, *num_devices,
                            device_list, NULL);

  // Create one OpenCL context for each device in the platform
  *context =
      clCreateContext(NULL, *num_devices, device_list, NULL, NULL, clStatus);
  // Create a command queue
  *command_queue =
      clCreateCommandQueueWithProperties(*context, device_list[0], 0, clStatus);
}*/

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

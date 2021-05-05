/*
Build: g++ cltest2.cpp -o cltest2 -lOpenCL
g++ -D_GLIBCXX_USE_CXX11_ABI=0 example.cpp -o example.out -lOpenCL
-lmysqlcppconn */
#include "head.hpp"
#include "timer_class.hpp"
#define PROGRAM_FILE "examp.cl"
#include <fstream>

#define LOCAL_SIZE 32
#define GLOBAL_SIZE 1024

int main(void)
{
  Timer timer;
  
	size_t kernel_length;
	string kernel_string;
	read_kernel_code(PROGRAM_FILE, &kernel_length, &kernel_string);

	Table1Type *t1 = NULL;
	int t1_size;
	load_database(&t1, &t1_size);

	size_t local_size;
	size_t global_size;
	int interval_size;
	size_calculator(&global_size, &local_size, &interval_size, t1_size);

	cl_int clStatus;
	cl_device_id device;
	cl_context context;
	cl_program program;
	cl_kernel kernel;
	cl_command_queue command_queue;

	device = create_device(GPU);
	context = clCreateContext(NULL, 1, &device, NULL, NULL, &clStatus);
	command_queue = clCreateCommandQueue(context, device, 0, &clStatus);

	int *result_counter = (int*) malloc(sizeof(int) *global_size);

	cl_mem Table1_clmem = clCreateBuffer(context, CL_MEM_READ_WRITE,t1_size* sizeof(Table1Type), NULL, &clStatus);
    cl_mem Table_size_clmem = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &clStatus);
	cl_mem Interval_size_clmem = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &clStatus);

	cl_mem TableResult_clmem = clCreateBuffer(context, CL_MEM_READ_WRITE, t1_size* sizeof(Table1Type), NULL, &clStatus);
	cl_mem Result_indexes_list_clmem = clCreateBuffer(context, CL_MEM_READ_WRITE, global_size* sizeof(int), NULL, &clStatus);

	clStatus = clEnqueueWriteBuffer(command_queue, Table1_clmem, CL_TRUE, 0, t1_size * sizeof(Table1Type), t1, 0,	NULL, NULL);
	clStatus = clEnqueueWriteBuffer(command_queue, Table_size_clmem, CL_TRUE, 0, sizeof(int), &t1_size, 0,	NULL, NULL);
	clStatus = clEnqueueWriteBuffer(command_queue, Interval_size_clmem, CL_TRUE, 0, sizeof(int), &interval_size, 0, NULL, NULL);

	// Create a program from the kernel source
	program =
		clCreateProgramWithSource(context, 1, (const char **) &kernel_string,
			(const size_t *) &kernel_length, &clStatus);

	clStatus = clBuildProgram(program, 1, &device, NULL, NULL, NULL);


	// Kernel letrehozasa
	kernel = clCreateKernel(program, "examp", &clStatus);

	clStatus =
		clSetKernelArg(kernel, 0, sizeof(cl_mem), (Table1Type*) &Table1_clmem);
	clStatus =
		clSetKernelArg(kernel, 1, sizeof(cl_mem), (Table1Type*) &TableResult_clmem);
	clStatus =
		clSetKernelArg(kernel, 2, sizeof(cl_mem), (int*) &Result_indexes_list_clmem);
	clStatus =
		clSetKernelArg(kernel, 3, sizeof(cl_mem), (int*) &Table_size_clmem);
	clStatus =
		clSetKernelArg(kernel, 4, sizeof(cl_mem), (int*) &Interval_size_clmem);

	clStatus = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
	clStatus = clFinish(command_queue);

  	clStatus =
		clEnqueueReadBuffer(command_queue, Result_indexes_list_clmem, CL_TRUE, 0,
			global_size * sizeof(int), result_counter, 0, NULL, NULL);

    ofstream myfile2;
    myfile2.open("outpuffer.csv");


timer.start();
int s=0;
  for(int ix=0; ix<global_size; ix++)
	s+=result_counter[ix];
TableResultType *result = (TableResultType*) malloc(sizeof(TableResultType) *s);
int count = 0;
for(int i=0; i< global_size; i++)
{
if(result_counter[i] < 1 ) continue; 

clStatus = clEnqueueReadBuffer(command_queue, TableResult_clmem, CL_TRUE, 
	i*interval_size * sizeof(TableResultType),
	result_counter[i]* sizeof(TableResultType), 
	&result[count], 0, NULL, NULL);
	count+=result_counter[i];
}

	clStatus = clFinish(command_queue);

cout << timer.elapsedMicroseconds() << endl;
	cout << s << endl; ;
    ofstream myfile;
    myfile.open("res.csv");


int index , i, j;

for(i=0; i < s; i++){

 	 myfile	<< t1[result[ i ].index].c1p1 << ","
			<< t1[result[ i ].index].c2 << ","
			<< t1[result[ i ].index].c3 << ","
			<< t1[result[ i ].index].c4 << endl;
}


  	clStatus = clReleaseKernel(kernel);
	clStatus = clReleaseProgram(program);
	clStatus = clReleaseContext(context);
	clStatus = clReleaseCommandQueue(command_queue);
	clStatus = clReleaseDevice(device);

	clStatus = clReleaseMemObject(Table1_clmem);
	clStatus = clReleaseMemObject(Table_size_clmem);
	clStatus = clReleaseMemObject(Interval_size_clmem);
  	clStatus = clReleaseMemObject(TableResult_clmem);
	clStatus = clReleaseMemObject(Result_indexes_list_clmem);

	free(t1);
	free(result_counter);
	free(result);

	return 0;

}

void read_kernel_code(char
	const *file, size_t *kernel_length,
	string *kernel_string)
{
	FILE * fp;
	fp = fopen(file, "r");
	if (!fp)
	{
		fprintf(stderr, "Error loading kernel.\n");
		exit(1);
	}
	fseek(fp, 0, SEEK_END);
	*kernel_length = ftell(fp);
	rewind(fp);
	char *kernel_str = (char*) malloc(*kernel_length);
	fread(kernel_str, 1, *kernel_length, fp);
	fclose(fp);
	*kernel_string = kernel_str;
}

void load_database(Table1Type **T1, int *T1_size)
{
	try
	{
		sql::Driver * driver;
		sql::Connection * con;
		sql::Statement * stmt;
		sql::ResultSet * res;
		sql::PreparedStatement * pstmt;

		driver = get_driver_instance();
		con = driver->connect("tcp://192.168.0.43:3306", "program", "a");

		con->setSchema("speed_test");

		pstmt = con->prepareStatement("SELECT *FROM speedtest_1048576 ");
		res = pstmt->executeQuery();

		int i = 0;
		*T1_size = res->rowsCount();
		*T1 = (Table1Type*) malloc(sizeof(Table1Type) * *T1_size);

		while (res->next())
		{
			T1[0][i].c1p1 = res->getInt("c1p1");
			T1[0][i].c2 = res->getInt("c2");
			T1[0][i].c3 = res->getInt("c3");
			T1[0][i].c4 = res->getInt("c4");
			i++;
		}
		delete res;
		delete pstmt;

	}
	catch (sql::SQLException &e)
	{

		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << ")" << endl;
		cout << "\n" << EXIT_FAILURE;
	}
}

cl_device_id create_device(const int gpu)
{

	cl_platform_id platform;
	cl_device_id dev;
	int err;

	/*Identify a platform */
	err = clGetPlatformIDs(1, &platform, NULL);
	if (err < 0)
	{
		perror("A platform nem azonosítható!");
		exit(1);
	}

	// Access a device
	if (gpu == 1)
	{
		// GPU
		err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
		if (err == CL_DEVICE_NOT_FOUND)
		{
			// CPU
			err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
		}
	}
	else
	{
		err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
	}

	if (err < 0)
	{
		perror("Egyetlen eszköz sem elérhető!");
		exit(1);
	}

	return dev;
}

void size_calculator(size_t *global, size_t *local, int *interval, int table_size)
{
	*local = LOCAL_SIZE;
	*global = GLOBAL_SIZE;
	
	if( table_size % *global == 0 )
	*interval = table_size / *global;
	else
	*interval = table_size / *global + 1;
}
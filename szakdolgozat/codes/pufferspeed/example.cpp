/*
Build: g++ cltest2.cpp -o cltest2 -lOpenCL
g++ -D_GLIBCXX_USE_CXX11_ABI=0 example.cpp -o example.out -lOpenCL
-lmysqlcppconn */
#include "head.hpp"
#include "timer_class.hpp"
#include <fstream>
#define PROGRAM_FILE "examp.cl"

int main(void)
{
  Timer timer;
	

	size_t kernel_length;
	string kernel_string;
	read_kernel_code(PROGRAM_FILE, &kernel_length, &kernel_string);


	Table1Type *t1 = NULL;
	int t1_size;
	load_database(&t1, &t1_size);


	size_t local_size = 8;
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



	//TableResultType *result = (TableResultType*) malloc(sizeof(TableResultType) *t1_size);
	int *result_counter = (int*) malloc(sizeof(int) *global_size);

	cl_mem Table1_clmem = clCreateBuffer(context, CL_MEM_READ_WRITE,t1_size* sizeof(Table1Type), NULL, &clStatus);
    cl_mem Table_size_clmem = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &clStatus);
	cl_mem Interval_size_clmem = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &clStatus);

	cl_mem TableResult_clmem = clCreateBuffer(context, CL_MEM_READ_WRITE, t1_size* sizeof(Table1Type), NULL, &clStatus);
	cl_mem Result_indexes_list_clmem = clCreateBuffer(context, CL_MEM_READ_WRITE, global_size* sizeof(int), NULL, &clStatus);


    ofstream myfile;
    myfile.open("inpuffer.csv");
	int sum;

int range=65536;
while( range <= 1048576){
sum = 0;
	for(int i = 0; i < 5; i++)
	{
	timer.start();
	clStatus = clEnqueueWriteBuffer(command_queue, Table1_clmem, CL_TRUE, 0, range* sizeof(Table1Type), t1, 0,	NULL, NULL);
	clStatus = clEnqueueWriteBuffer(command_queue, Table_size_clmem, CL_TRUE, 0, sizeof(int), &t1_size, 0,	NULL, NULL);
	clStatus = clEnqueueWriteBuffer(command_queue, Interval_size_clmem, CL_TRUE, 0, sizeof(int), &interval_size, 0, NULL, NULL);
	clStatus = clFinish(command_queue);
	sum += timer.elapsedMicroseconds();
	}
range += 65536;
myfile << sum/5 << endl;
}
myfile.close();

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

    myfile.open("outpuffer2.csv");

	sum =0;
	timer.start();
   clStatus =
       clEnqueueReadBuffer(command_queue, Result_indexes_list_clmem, CL_TRUE, 0,
           global_size* sizeof(int), result_counter, 0, NULL, NULL);

for(int k = 0; k<global_size; k++) sum+=result_counter[k];
cout << sum << endl;
TableResultType *result = (TableResultType*) malloc(sizeof(TableResultType) *sum);
int count = 0;
for(int i=0; i< global_size; i++)
 {
   if(result_counter[i] > 0)
   clStatus = clEnqueueReadBuffer(command_queue, TableResult_clmem, CL_TRUE, i*global_size * sizeof(TableResultType),
       result_counter[i]* sizeof(TableResultType), &result[count]  , 0,
       NULL, NULL);
	 count+=result_counter[i];
 }
 clStatus = clFinish(command_queue);
cout << timer.elapsedMicroseconds() << endl;

	//for(int i = 0; i< sum; i++) myfile << t1[ result[i].index ].c1p1 << endl;


/*range = 65536;			
while( range <= 1048576){

sum = 0;
	for(int i = 0; i < 5; i++)
	{
	timer.start();

	  	clStatus =
		clEnqueueReadBuffer(command_queue, Result_indexes_list_clmem, CL_TRUE, 0,
			global_size* sizeof(int), result_counter, 0, NULL, NULL);

	clStatus = clEnqueueReadBuffer(command_queue, TableResult_clmem, CL_TRUE, 0,
		range * sizeof(TableResultType), result  , 0,
		NULL, NULL);

	clStatus = clFinish(command_queue);
	sum += timer.elapsedMicroseconds();
	}
range += 65536;
myfile << sum/5 << endl;
}*/



  

  	clStatus = clFlush(command_queue);

 /* for(int i = 0; i< sum; i++)
  {
    cout << t1[result[ i ].index].c1p1 << endl;
  }*/

/*
for(i=0; i< global_size; i++)
{

  for(j=0; j<result_counter[i]; j++)
  cout << t1[result[ i*global_size + j ].index].c1p1 << endl;

}*/



	return 0;

	
	  clStatus = clReleaseKernel(kernel);
	  clStatus = clReleaseProgram(program);
	  clStatus = clReleaseContext(context);
	  clStatus = clReleaseCommandQueue(command_queue);
	  clStatus = clReleaseDevice(device);

	  clStatus = clReleaseMemObject(Table1_clmem);
	  clStatus = clReleaseMemObject(TableResult_clmem);
	  clStatus = clReleaseMemObject(Result_indexes_list_clmem);


	 /* free(t1);
	  free(tout);
	  free(returned_rows);
	  free(limit);
	  free(range); */

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


		pstmt = con->prepareStatement("SELECT * FROM speedtest_1048576");
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
		perror("Couldn't identify a platform");
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
		perror("Couldn't access any devices");
		exit(1);
	}

	return dev;
}

void size_calculator(size_t *global, size_t *local, int *interval, int table_size)
{

	*interval = sqrt(table_size);
	if ((*interval % *local) != 0)
		*global = *interval + (*local - (*interval % *local));
	else *global = *interval;

	//cout << *global << "; " << *interval << endl;
}
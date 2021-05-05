/*
Build: g++ cltest2.cpp -o cltest2 -lOpenCL
g++ -D_GLIBCXX_USE_CXX11_ABI=0 example.cpp -o example.out -lOpenCL
-lmysqlcppconn */

#include "head.hpp"
#include "timer_class.hpp"
#include <fstream>
#include <unistd.h>
#include <math.h>

#define PROGRAM_FILE "examp.cl"
#define GPU 1
#define CPU 0

#define LOCAL_SIZE 32
#define GLOBAL_SIZE 1024

int main(void)
{

    Timer timer;
timer.start();
    //Kernel kod beolvasasa
    size_t kernel_length;
    string kernel_string;
    read_kernel_code(PROGRAM_FILE, &kernel_length, &kernel_string);

    //Tablak es mereteik definialasa
    TableMType* T_m = NULL;
    int t_m_size;
    TableSType* T_s = NULL;
    int t_s_size;

    //Tablak es mereteik meghatarozasa
    load_database(&T_m, &t_m_size, &T_s, &t_s_size);

    //Item meretek meghatarozasa
    int TM_interval_size;
    int TS_interval_size;

    // Local méret megadasa
    size_t TM_local_size;
    size_t TM_global_size;

    size_t TS_local_size;
    size_t TS_global_size;

    //globalis meret es munkaelemek osszvonasa
    size_calculator(&TM_global_size, &TM_local_size, &TM_interval_size, t_m_size);
    size_calculator(&TS_global_size, &TS_local_size, &TS_interval_size, t_s_size);

    //OpenCL objektumok
    cl_int clStatus;
    cl_device_id device;
    cl_context context;
    cl_program program;
    cl_kernel kernel;
    cl_kernel kernel2;
    cl_command_queue command_queue;
    cl_command_queue command_queue2;

    //eszkoz letrehozas
    device = create_device(GPU);

    // OpenCL kontextus letrehozasqa eszkozonkent
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &clStatus);

    // Parancssor letrehozasa
    command_queue = clCreateCommandQueue(context, device, 0, &clStatus);
    command_queue2 = clCreateCommandQueue(context, device, 0, &clStatus);

    //Vegeremeny (jelenesetben index parok + egy kalkulalt ertek)
    TableOutType* cl_query_ansver = (TableOutType*)malloc(sizeof(TableOutType) * t_m_size);
    int* Final_Counter = (int*)malloc(sizeof(int) * TM_global_size);

    //M lekerdezeshez
    cl_mem TableM_clmem = clCreateBuffer(context, CL_MEM_READ_WRITE,
        t_m_size * sizeof(TableMType), NULL, &clStatus);
    cl_mem M_TableOUT_clmem = clCreateBuffer(context, CL_MEM_READ_WRITE,
        t_m_size * sizeof(int), NULL, &clStatus);

    cl_mem TM_Returned_Rows_clmem = clCreateBuffer(
        context, CL_MEM_READ_WRITE, TM_global_size * sizeof(int), NULL, &clStatus);

    cl_mem TM_Limit_clmem = clCreateBuffer(
        context, CL_MEM_READ_WRITE, sizeof(int), NULL, &clStatus);
    cl_mem TM_Interval_size_clmem = clCreateBuffer(
        context, CL_MEM_READ_WRITE, sizeof(int), NULL, &clStatus);

    //S lekerdezeshez
    cl_mem TableS_clmem = clCreateBuffer(context, CL_MEM_READ_WRITE,
        t_s_size * sizeof(TableSType), NULL, &clStatus);
    cl_mem S_TableOUT_clmem = clCreateBuffer(context, CL_MEM_READ_WRITE,
        t_s_size * sizeof(TableOutType), NULL, &clStatus);

    cl_mem TS_Returned_Rows_clmem = clCreateBuffer(
        context, CL_MEM_READ_WRITE, TS_global_size * sizeof(int), NULL, &clStatus);

    cl_mem TS_Limit_clmem = clCreateBuffer(
        context, CL_MEM_READ_WRITE, sizeof(int), NULL, &clStatus);
    cl_mem TS_Interval_size_clmem = clCreateBuffer(
        context, CL_MEM_READ_WRITE, sizeof(int), NULL, &clStatus);
    cl_mem TS_Global_clmem = clCreateBuffer(
        context, CL_MEM_READ_WRITE, sizeof(int), NULL, &clStatus);

    //Vegleges kernelhez
    cl_mem Final_Resoult_clmem = clCreateBuffer(
        context, CL_MEM_READ_WRITE, t_m_size * sizeof(TableOutType), NULL, &clStatus);
    cl_mem Final_Counter_clmem = clCreateBuffer(
        context, CL_MEM_READ_WRITE, TM_global_size * sizeof(int), NULL, &clStatus);
    //Tablak masolasa pufferekbe
    clStatus = clEnqueueWriteBuffer(command_queue, TableM_clmem, CL_TRUE, 0,
        t_m_size * sizeof(TableMType), T_m, 0,
        NULL, NULL);
    clStatus = clEnqueueWriteBuffer(command_queue, TM_Limit_clmem, CL_TRUE, 0,
        sizeof(int), &t_m_size, 0,
        NULL, NULL);
    clStatus = clEnqueueWriteBuffer(command_queue, TM_Interval_size_clmem, CL_TRUE, 0,
        sizeof(int), &TM_interval_size, 0,
        NULL, NULL);

    clStatus = clEnqueueWriteBuffer(command_queue, TableS_clmem, CL_TRUE, 0,
        t_s_size * sizeof(TableSType), T_s, 0,
        NULL, NULL);
    clStatus = clEnqueueWriteBuffer(command_queue, TS_Limit_clmem, CL_TRUE, 0,
        sizeof(int), &t_s_size, 0,
        NULL, NULL);
    clStatus = clEnqueueWriteBuffer(command_queue, TS_Interval_size_clmem, CL_TRUE, 0,
        sizeof(int), &TS_interval_size, 0,
        NULL, NULL);
    clStatus = clEnqueueWriteBuffer(command_queue, TS_Global_clmem, CL_TRUE, 0,
        sizeof(int), &TS_global_size, 0,
        NULL, NULL);    

    // Program letrehozasa a ekrnel kodbol
    program = clCreateProgramWithSource(context, 1, (const char**)&kernel_string,
        (const size_t*)&kernel_length, &clStatus);

    // Program felepitese
    clStatus = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    // Elso kernel letrehozasa

    kernel = clCreateKernel(program, "examp", &clStatus);
    kernel2 = clCreateKernel(program, "examp2", &clStatus);

    cout<< timer.elapsedMicroseconds() << endl;
    timer.start();

    //argumentumok beallitasa
    clStatus = clSetKernelArg(kernel, 0, sizeof(cl_mem), (TableMType*)&TableM_clmem);
    clStatus = clSetKernelArg(kernel, 1, sizeof(cl_mem), (int*)&M_TableOUT_clmem);
    clStatus = clSetKernelArg(kernel, 2, sizeof(cl_mem), (int*)&TM_Returned_Rows_clmem);
    clStatus = clSetKernelArg(kernel, 3, sizeof(cl_mem), (int*)&TM_Limit_clmem);
    clStatus = clSetKernelArg(kernel, 4, sizeof(cl_mem), (int*)&TM_Interval_size_clmem);
    //argumentumok beallitasa
    clStatus = clSetKernelArg(kernel2, 0, sizeof(cl_mem), (TableSType*)&TableS_clmem);
    clStatus = clSetKernelArg(kernel2, 1, sizeof(cl_mem), (int*)&S_TableOUT_clmem);
    clStatus = clSetKernelArg(kernel2, 2, sizeof(cl_mem), (int*)&TS_Returned_Rows_clmem);
    clStatus = clSetKernelArg(kernel2, 3, sizeof(cl_mem), (int*)&TS_Limit_clmem);
    clStatus = clSetKernelArg(kernel2, 4, sizeof(cl_mem), (int*)&TS_Interval_size_clmem);
    //Kernelek futtatasa

    clStatus = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &TM_global_size, &TM_local_size, 0, NULL, NULL);
    clStatus = clEnqueueNDRangeKernel(command_queue2, kernel2, 1, NULL, &TS_global_size, &TS_local_size, 0, NULL, NULL);
    //Varakozas a kernelekre
    clStatus = clFinish(command_queue);
    clStatus = clFinish(command_queue2);

cout<< timer.elapsedMicroseconds() << endl;
timer.start();
  //  cout<< timer.elapsedMicroseconds() << endl;
//timer.start();

//befejezo kernel elkeszitese
    kernel = clCreateKernel(program, "exampfinal", &clStatus);
  //  cout<< timer.elapsedMicroseconds() << endl;
//timer.start();

//argumentumok betotese
    clStatus = clSetKernelArg(kernel, 0, sizeof(cl_mem), (TableMType*)&TableM_clmem);
    clStatus = clSetKernelArg(kernel, 1, sizeof(cl_mem), (int*)&M_TableOUT_clmem);
    clStatus = clSetKernelArg(kernel, 2, sizeof(cl_mem), (int*)&TM_Returned_Rows_clmem);
    clStatus = clSetKernelArg(kernel, 3, sizeof(cl_mem), (int*)&TM_Interval_size_clmem);

    clStatus = clSetKernelArg(kernel, 4, sizeof(cl_mem), (TableSType*)&TableS_clmem);
    clStatus = clSetKernelArg(kernel, 5, sizeof(cl_mem), (int*)&S_TableOUT_clmem);
    clStatus = clSetKernelArg(kernel, 6, sizeof(cl_mem), (int*)&TS_Returned_Rows_clmem);
    clStatus = clSetKernelArg(kernel, 7, sizeof(cl_mem), (int*)&TS_Interval_size_clmem);
    clStatus = clSetKernelArg(kernel, 8, sizeof(cl_mem), (int*)&TS_Global_clmem);

    clStatus = clSetKernelArg(kernel, 9, sizeof(cl_mem), (TableOutType*)&Final_Resoult_clmem);
    clStatus = clSetKernelArg(kernel, 10, sizeof(cl_mem), (int*)&Final_Counter_clmem);

    TM_local_size = 16;
    clStatus = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
        &TM_global_size, &TM_local_size, 0, NULL, NULL);
            clStatus = clFinish(command_queue);
 cout<< timer.elapsedMicroseconds() << endl;

    //eredmeny kiolvasasa
timer.start();
    clStatus = clEnqueueReadBuffer(command_queue, Final_Resoult_clmem, CL_TRUE, 0,
        sizeof(TableOutType) * t_m_size, cl_query_ansver, 0, NULL, NULL);

    clStatus = clEnqueueReadBuffer(command_queue, Final_Counter_clmem, CL_TRUE, 0,
        sizeof(int) * TM_global_size, Final_Counter, 0, NULL, NULL);
    clStatus = clFinish(command_queue);
cout << timer.elapsedMicroseconds() << endl;

    clStatus = clFlush(command_queue);

timer.start();
    ofstream myfile;
    myfile.open("res.csv");
int index;
for(int i = 0; i < TM_global_size; i++)
  {
    for(int j=0; j< Final_Counter[i]; j++ )
    {
     index = i*TM_interval_size + j;
     myfile <<cl_query_ansver[ index ].M_index << ","
		<< cl_query_ansver[ index ].S_index << ","
		<< cl_query_ansver[ index ].MC3xSC3 
		<< endl;
      
    }
  }
cout << timer.elapsedMicroseconds() << endl;

    clStatus = clReleaseKernel(kernel);
    clStatus = clReleaseProgram(program);
    clStatus = clReleaseContext(context);
    clStatus = clReleaseCommandQueue(command_queue);
    clStatus = clReleaseDevice(device);

    clStatus = clReleaseMemObject(TableM_clmem);
    clStatus = clReleaseMemObject(M_TableOUT_clmem);
    clStatus = clReleaseMemObject(TM_Returned_Rows_clmem);
    clStatus = clReleaseMemObject(TM_Limit_clmem);
    clStatus = clReleaseMemObject(TM_Interval_size_clmem);

    
    clStatus = clReleaseMemObject(TableS_clmem);
    clStatus = clReleaseMemObject(S_TableOUT_clmem);
    clStatus = clReleaseMemObject(TS_Global_clmem);
    clStatus = clReleaseMemObject(TS_Limit_clmem);
    clStatus = clReleaseMemObject(TS_Interval_size_clmem);
    clStatus = clReleaseMemObject(TS_Returned_Rows_clmem);

    clStatus = clReleaseMemObject(Final_Resoult_clmem);
    clStatus = clReleaseMemObject(Final_Counter_clmem);

    free(T_m);
    free(T_s);
    free(cl_query_ansver);


}

void read_kernel_code(char const* file, size_t* kernel_length,
    string* kernel_string)
{
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

void load_database(TableMType** TM, int* TM_size, TableSType** TS, int* TS_size)
{
        Timer timer;
    try {
        sql::Driver* driver;
        sql::Connection* con;
        sql::Statement* stmt;
        sql::ResultSet* res;
        sql::PreparedStatement* pstmt;

        /* Create a connection */

        driver = get_driver_instance();
        con = driver->connect("tcp://192.168.0.43:3306", "program", "a");
        /* Connect to the MySQL test database */
        con->setSchema("speed2");

        //1
        pstmt = con->prepareStatement("SELECT * FROM speed_262144");
        res = pstmt->executeQuery();
        int i = 0;
        *TM_size = res->rowsCount();
        *TM = (TableMType*)malloc(sizeof(TableMType) * *TM_size);

        while (res->next()) {
            TM[0][i].c1p1 = res->getInt("c1p1");
          //  TM[0][i].c2 = res->getInt("c2");
            TM[0][i].c3 = res->getInt("c3");
            TM[0][i].fk_s = res->getInt("fk_s");
           // TM[0][i].fk_xs = res->getInt("fk_xs");
            i++;
        }

        delete res;
        delete pstmt;
        //2
        pstmt = con->prepareStatement("SELECT * FROM speed_131072");
        res = pstmt->executeQuery();
        i = 0;
        *TS_size = res->rowsCount();
        *TS = (TableSType*)malloc(sizeof(TableSType) * *TS_size);

        while (res->next()) {
            TS[0][i].c1p1 = res->getInt("c1p1");
          //  TS[0][i].c2 = res->getInt("c2");
            TS[0][i].c3 = res->getInt("c3");
          //  TS[0][i].c4 = res->getInt("c4");
          //  TS[0][i].fk_xs = res->getInt("fk_xs");
            i++;
        }
        delete res;
        delete pstmt;
        delete con;
    }
    catch (sql::SQLException& e) {

        cout << "# ERR: SQLException in " << __FILE__;
        cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
        cout << "# ERR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << " )" << endl;

        cout << "\n" << EXIT_FAILURE;
    }
    // cout << "Full query completed.\n" << endl;
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

void size_calculator(size_t* global, size_t* local, int* interval, int table_size)
{

	*local = LOCAL_SIZE;
	*global = GLOBAL_SIZE;
	
	if( table_size % *global == 0 )
	*interval = table_size / *global;
	else
	*interval = table_size / *global + 1;
}
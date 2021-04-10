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
#define VECTOR_SIZE1 1048576
#define GPU 1
#define CPU 0

int main(void)
{

    Timer timer;

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
    int* TM_item_range = (int*)malloc(sizeof(int));
    int* TS_item_range = (int*)malloc(sizeof(int));

    // Local méret megadasa
    size_t TM_local_size = 128;
    size_t TM_global_size;

    size_t TS_local_size = 32;
    size_t TS_global_size;

    //globalis meret es munkaelemek osszvonasa
    size_calculator(&TM_global_size, &TM_local_size, TM_item_range, t_m_size);
    size_calculator(&TS_global_size, &TS_local_size, TS_item_range, t_s_size);

    cout << *TM_item_range << "\t" << TM_global_size << endl;

    //OpenCL objektumok
    cl_int clStatus;
    cl_device_id device;
    cl_context context;
    cl_program program;
    cl_kernel kernel;
    cl_command_queue command_queue;

    //eszkoz letrehozas
    device = create_device(GPU);

    // OpenCL kontextus letrehozasqa eszkozonkent
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &clStatus);

    // Parancssor letrehozasa
    command_queue = clCreateCommandQueue(context, device, 0, &clStatus);

    //Vegeremeny (jelenesetben index parok + egy kalkulalt ertek)
    TableOutType* tout = (TableOutType*)malloc(sizeof(TableOutType) * t_m_size);
    int* Final_Counter = (int*)malloc(sizeof(int) * TM_global_size);

    //Hataerertek a tablakhoz tartozo kernelnek
    int* limitTM = (int*)malloc(sizeof(int));
    *limitTM = t_m_size;

    int* limitTS = (int*)malloc(sizeof(int));
    *limitTS = t_s_size;

    //M lekerdezeshez
    cl_mem TableM_clmem = clCreateBuffer(context, CL_MEM_READ_WRITE,
        t_m_size * sizeof(TableMType), NULL, &clStatus);
    cl_mem M_TableOUT_clmem = clCreateBuffer(context, CL_MEM_READ_WRITE,
        t_m_size * sizeof(int), NULL, &clStatus);
    cl_mem TM_Returned_Rows_clmem = clCreateBuffer(
        context, CL_MEM_READ_WRITE, TM_global_size * sizeof(int), NULL, &clStatus);
    cl_mem TM_Limit_clmem = clCreateBuffer(
        context, CL_MEM_READ_WRITE, sizeof(int), NULL, &clStatus);
    cl_mem TM_Item_range_clmem = clCreateBuffer(
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
    cl_mem TS_Item_range_clmem = clCreateBuffer(
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
    clStatus = clEnqueueWriteBuffer(command_queue, TM_Item_range_clmem, CL_TRUE, 0,
        sizeof(int), TM_item_range, 0,
        NULL, NULL);

    clStatus = clEnqueueWriteBuffer(command_queue, TableS_clmem, CL_TRUE, 0,
        t_s_size * sizeof(TableSType), T_s, 0,
        NULL, NULL);
    clStatus = clEnqueueWriteBuffer(command_queue, TS_Limit_clmem, CL_TRUE, 0,
        sizeof(int), &t_s_size, 0,
        NULL, NULL);
    clStatus = clEnqueueWriteBuffer(command_queue, TS_Item_range_clmem, CL_TRUE, 0,
        sizeof(int), TS_item_range, 0,
        NULL, NULL);

    // Program letrehozasa a ekrnel kodbol
    program = clCreateProgramWithSource(context, 1, (const char**)&kernel_string,
        (const size_t*)&kernel_length, &clStatus);

    // Program felepitese
    clStatus = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    timer.start();
    // Elso kernel letrehozasa
    kernel = clCreateKernel(program, "examp", &clStatus);

    //argumentumok beallitasa
    clStatus = clSetKernelArg(kernel, 0, sizeof(cl_mem), (TableMType*)&TableM_clmem);
    clStatus = clSetKernelArg(kernel, 1, sizeof(cl_mem), (int*)&M_TableOUT_clmem);
    clStatus = clSetKernelArg(kernel, 2, sizeof(cl_mem), (int*)&TM_Returned_Rows_clmem);
    clStatus = clSetKernelArg(kernel, 3, sizeof(cl_mem), (int*)&TM_Limit_clmem);
    clStatus = clSetKernelArg(kernel, 4, sizeof(cl_mem), (int*)&TM_Item_range_clmem);

    cout << "indulni kene\n";
    //Kernel futtatas
    clStatus = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &TM_global_size, &TM_local_size, 0, NULL, NULL);

    //Masodik kernel letrehozasa
    kernel = clCreateKernel(program, "examp2", &clStatus);

    clStatus = clSetKernelArg(kernel, 0, sizeof(cl_mem), (TableSType*)&TableS_clmem);
    clStatus = clSetKernelArg(kernel, 1, sizeof(cl_mem), (int*)&S_TableOUT_clmem);
    clStatus = clSetKernelArg(kernel, 2, sizeof(cl_mem), (int*)&TS_Returned_Rows_clmem);
    clStatus = clSetKernelArg(kernel, 3, sizeof(cl_mem), (int*)&TS_Limit_clmem);
    clStatus = clSetKernelArg(kernel, 4, sizeof(cl_mem), (int*)&TS_Item_range_clmem);

    clStatus = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &TS_global_size, &TS_local_size, 0, NULL, NULL);

    clStatus = clFinish(command_queue);
    clStatus = clFlush(command_queue);

    kernel = clCreateKernel(program, "exampfinal", &clStatus);

    clStatus = clSetKernelArg(kernel, 0, sizeof(cl_mem), (TableMType*)&TableM_clmem);
    clStatus = clSetKernelArg(kernel, 1, sizeof(cl_mem), (int*)&M_TableOUT_clmem);
    clStatus = clSetKernelArg(kernel, 2, sizeof(cl_mem), (int*)&TM_Returned_Rows_clmem);

    clStatus = clSetKernelArg(kernel, 3, sizeof(cl_mem), (TableSType*)&TableS_clmem);
    clStatus = clSetKernelArg(kernel, 4, sizeof(cl_mem), (int*)&S_TableOUT_clmem);
    clStatus = clSetKernelArg(kernel, 5, sizeof(cl_mem), (int*)&TS_Returned_Rows_clmem);
    clStatus = clSetKernelArg(kernel, 6, sizeof(cl_mem), (int*)&TS_Limit_clmem);

    clStatus = clSetKernelArg(kernel, 7, sizeof(cl_mem), (TableOutType*)&Final_Resoult_clmem);
    clStatus = clSetKernelArg(kernel, 8, sizeof(cl_mem), (int*)&Final_Counter_clmem);

    TM_local_size = 16;

    clStatus = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
        &TM_global_size, &TM_local_size, 0, NULL, NULL);

    clStatus = clEnqueueReadBuffer(command_queue, Final_Resoult_clmem, CL_TRUE, 0,
        sizeof(TableOutType) * t_m_size, tout, 0, NULL, NULL);
    clStatus = clEnqueueReadBuffer(command_queue, Final_Counter_clmem, CL_TRUE, 0,
        sizeof(int) * TM_global_size, Final_Counter, 0, NULL, NULL);

    clStatus = clFinish(command_queue);
    clStatus = clFlush(command_queue);

    //timer.stop();
    //cout << timer.elapsedMicroseconds() << ",";

    int i, k = 0;

    for (i = 0; i < 512; i++) {
        k += Final_Counter[i];
    }

    timer.stop();
    cout << timer.elapsedMicroseconds() << ",";

    cout << k << endl;

    /*for(i = 0; i < TM_global_size; i++)
  {
    for(k=0; k< number_of_return_indexs_TM[i]; k++ )
    {
      printf("%d | %d | %d | %d | %d \n", T_m[  tout[ i* TM_global_size +k ].M  ].c1p1, 
      T_m[  tout[ i*TM_global_size +k ].M  ].c2, T_m[  tout[ i*TM_global_size +k ].M  ].c3, 
      T_m[  tout[ i*TM_global_size +k ].M  ].fk_s, T_m[  tout[ i*TM_global_size +k ].M  ].fk_xs );
    }
  }*/

    clStatus = clReleaseKernel(kernel);
    clStatus = clReleaseProgram(program);
    clStatus = clReleaseContext(context);
    clStatus = clReleaseCommandQueue(command_queue);
    clStatus = clReleaseDevice(device);

    /*clStatus = clReleaseMemObject(Table1_clmem);
  clStatus = clReleaseMemObject(TableOUT_clmem);
  clStatus = clReleaseMemObject(Returned_Rows_clmem);
  clStatus = clReleaseMemObject(Range_clmem);
  clStatus = clReleaseMemObject(Limit_clmem);*/

    /*free(tout);
  free(returned_row_indexs);
  free(limit);
  free(range);*/

    /*}
   // myfile << endl;
  }*/

    //free(t1);
    return 0;
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
            TM[0][i].c2 = res->getInt("c2");
            TM[0][i].c3 = res->getInt("c3");
            TM[0][i].fk_s = res->getInt("fk_s");
            TM[0][i].fk_xs = res->getInt("fk_xs");
            i++;
        }
        cout << "van:" << i << endl;
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
            TS[0][i].c2 = res->getInt("c2");
            TS[0][i].c3 = res->getInt("c3");
            TS[0][i].c4 = res->getInt("c4");
            TS[0][i].fk_xs = res->getInt("fk_xs");
            i++;
        }
        cout << "van:" << i << endl;

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

    /* Identify a platform */
    err = clGetPlatformIDs(1, &platform, NULL);
    if (err < 0) {
        perror("Couldn't identify a platform");
        exit(1);
    }

    // Access a device
    if (gpu == 1) {
        // GPU
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
        if (err == CL_DEVICE_NOT_FOUND) {
            // CPU
            err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
        }
    }
    else {
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
    }

    if (err < 0) {
        perror("Couldn't access any devices");
        exit(1);
    }

    return dev;
}

void size_calculator(size_t* global, size_t* local, int* range, int table_size)
{

    *range = sqrt(table_size);
    if ((*range % *local) != 0)
        *global = *range + (*local - (*range % *local));
    else
        *global = *range;

    cout << *global << "; " << *range << endl;
}
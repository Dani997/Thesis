#include <CL/cl.hpp>
#include <stdio.h>
#include <math.h>

#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

using namespace std;

#define GPU 1
#define CPU 0

typedef struct {
  int c1p1;
  int c2;
  int c3;
  int c4;
} Table1Type;

typedef struct {
  int index;

} TableResultType;

void load_database(Table1Type **T1, int *T1_size);

void read_kernel_code(char const *file, size_t *kernel_length,
                      string *kernel_string);


cl_device_id create_device(const int gpu);

void size_calculator(size_t* global, size_t* local, int* interval, int table_size);

int counter_totalizer(int global_size, int *result_counter);


void write_file(int *result_counter, Table1Type *t1, TableResultType *result, int interval_size, int global_size, string file);
void write_file2(int s, Table1Type *t1, TableResultType *result, string file);


void cp_from_buffer(cl_int clStatus, cl_command_queue command_queue, int global_size, cl_mem TableResult_clmem,
					TableResultType *result, int *result_counter, int interval_size);




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
    int calc1;
  int calc2;

} TableResultType;

void load_database(Table1Type **T1, int *T1_size);

void read_kernel_code(char const *file, size_t *kernel_length,
                      string *kernel_string);


cl_device_id create_device(const int gpu);



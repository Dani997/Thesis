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

typedef struct {
  int c1p1;
  int c2;
  int c3;
  int fk_s;
  int fk_xs;
} TableMType;

typedef struct {
  int c1p1;
  int c2;
  int c3;
  int c4;
  int fk_xs;
} TableSType;



typedef struct {
  int M_index;
  int S_index;
  int MC2xSC3;
} TableOutType;

void load_database(TableMType **TM, int *TM_size, TableSType **TS, int *TS_size);

void read_kernel_code(char const *file, size_t *kernel_length,
                      string *kernel_string);


cl_device_id create_device(const int gpu);

void size_calculator(size_t *global, size_t *local, int *range, int table_size);


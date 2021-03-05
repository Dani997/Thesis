#include <CL/cl.h>
#include <CL/cl.hpp>
#include <bits/stdint-intn.h>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
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

// Tables

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

void read_kernel_code(char const *file, size_t *kernel_length, string *kernel_string);


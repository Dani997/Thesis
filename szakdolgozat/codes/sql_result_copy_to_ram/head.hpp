
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
  int c4;
} Table1Type;



void load_database(Table1Type **T1, int *T1_size);








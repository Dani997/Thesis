
/* g++ -D_GLIBCXX_USE_CXX11_ABI=0 dbtest.cpp -o  dbtest.out -lmysqlcppconn */
#include <iostream>
#include <stdlib.h>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <string.h>
#include <sys/cdefs.h>
#include <fstream>
#include "timer_class.hpp"

int main(void) {

 Timer timer;
 double time;
  
  timer.start();
  try {
    sql::Driver *driver;
    sql::Connection *con;
    sql::Statement *stmt;
    sql::ResultSet *res;
    sql::PreparedStatement *pstmt;


    driver = get_driver_instance();
    con = driver->connect("tcp://192.168.0.43:3306", "program", "a");

    con->setSchema("speed_test");
    timer.stop();
    time =  timer.elapsedMicroseconds();
    printf("A kapcsolat %lf microsec alatt letrejott.\n", time);

    string command;
    int i=32768,j;
    int range = 32768;
    int sum;

    ofstream myfile;
    myfile.open("all_select.csv");

    while(i <= 1048576){
      sum=0;
    command = "SELECT * FROM speedtest_1048576 Limit " + to_string(i);

    for(int j=0; j<5; j++){
    timer.start();

    pstmt = con->prepareStatement(command);
    res = pstmt->executeQuery();
    sum += timer.elapsedMicroseconds();  
    }
    i+=range;
      
    myfile << sum/5 << endl;

    delete res;
    delete pstmt;

    }


  } catch (sql::SQLException &e) {

    cout << "# ERR: SQLException in " << __FILE__;
    cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;

    cout << "# ERR: " << e.what();
    cout << " (MySQL error code: " << e.getErrorCode();
    cout << ", SQLState: " << e.getSQLState() << " )" << endl;

    return EXIT_FAILURE;
  }

  cout << "Done." << endl;
  return EXIT_SUCCESS;
}

/* g++ -D_GLIBCXX_USE_CXX11_ABI=0 mysqlspeedtest.cpp -o  mysqlspeedtest.out
 * -lmysqlcppconn */
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>

#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <string>
#include <sys/cdefs.h>

using namespace std;
using namespace std::chrono;

int main(void) {
  try {

    //---SPEED TEST START---
    auto start = high_resolution_clock::now();
    //----------------------
    sql::Driver *driver;
    sql::Connection *con;
    sql::Statement *stmt;
    sql::ResultSet *res;
    sql::PreparedStatement *pstmt;

    /* Create a connection */
    driver = get_driver_instance();
    con = driver->connect("tcp://192.168.0.43:3306", "program", "a");
    /* Connect to the MySQL test database */
    con->setSchema("speed_test");
    //---SPEED TEST START---
    auto stop = high_resolution_clock::now();
    //----------------------
    auto duration = duration_cast<microseconds>(stop - start);
    cout << duration.count() << endl;
    string s;
    int sum = 0;

    int i, k;
   /* ofstream myfile;
    myfile.open("c1p1_c2_c3_c4_select.csv");
    for (k = 1; k <= 128; k++) {
      s = "SELECT c1p1, c2, c3, c4 FROM speedtest_1048576 LIMIT " +
          to_string(k * 8192);
      sum = 0;
      for (i = 0; i < 1; i++) {*/
        //usleep(10);
        //---SPEED TEST START---
         start = high_resolution_clock::now();
        //----------------------

        s = "SELECT c1p1, c2, c3, c4 FROM speedtest_1048576";
        pstmt = con->prepareStatement(s);
        res = pstmt->executeQuery();

        //---SPEED TEST START---
         stop = high_resolution_clock::now();
        //----------------------
         duration = duration_cast<microseconds>(stop - start);
        delete res;
        delete pstmt;
        sum += duration.count();
    /*  }
      cout << k * 8192 << ", \t" << sum / 1 << "\n";
    }
    myfile.close();*/

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

  return 0;
}
/*
Build: g++ -D_GLIBCXX_USE_CXX11_ABI=0 dbtofile.cpp -o  dbtofile.out -lmysqlcppconn
*/
#include <fstream>
#include <iostream>
#include <stdlib.h>

#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

using namespace std;
// using namespace std::chrono;

int main(void) {
  try {
    sql::Driver *driver;
    sql::Connection *con;
    sql::Statement *stmt;
    sql::ResultSet *res;
    sql::PreparedStatement *pstmt;

    /* Create a connection */
    driver = get_driver_instance();
    con = driver->connect("tcp://192.168.0.43:3306", "program", "a");
    /* Connect to the MySQL test database */
    con->setSchema("thesis");

    ofstream myfile;

    pstmt = con->prepareStatement("SELECT * FROM T1");
    res = pstmt->executeQuery();

    myfile.open("t1.txt");
    res->afterLast();
    while (res->previous())
      myfile << res->getInt("c1p1") << " " << res->getInt("c2") << " "
             << res->getInt("c3") << " " << res->getInt("c4") << endl;
    myfile.close();
    delete res;

    pstmt = con->prepareStatement("SELECT * FROM T2");
    res = pstmt->executeQuery();

    myfile.open("t2.txt");
    res->afterLast();
    while (res->previous())
      myfile << res->getInt("c1p2") << " " << res->getInt("c2") << " "
             << res->getInt("c3") << " " << res->getInt("fk_p1_p2") << endl;
    myfile.close();
    delete res;

    pstmt = con->prepareStatement("SELECT * FROM T3");
    res = pstmt->executeQuery();

    myfile.open("t3.txt");
    res->afterLast();
    while (res->previous())
      myfile << res->getInt("c1p3") << " " << res->getInt("c2") << " "
             << res->getInt("c3") << " " << res->getInt("c4") << " "
             << res->getInt("fk_p1_p3") << " " << res->getInt("fk_p2_p3")
             << endl;
    myfile.close();
    delete res;

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
/*
Build: g++ cltest2.cpp -o cltest2 -lOpenCL
g++ -D_GLIBCXX_USE_CXX11_ABI=0 example.cpp -o example.out -lOpenCL
-lmysqlcppconn */
#include "head.hpp"
#include "timer_class.hpp"
#include <fstream>

int main(void)
{
  Timer timer;
	
timer.start();

	Table1Type *t1 = NULL;
	int t1_size;
	load_database(&t1, &t1_size);

	return 0;

}


void load_database(Table1Type **T1, int *T1_size)
{
	try
	{

	Timer timer2;

		sql::Driver * driver;
		sql::Connection * con;
		sql::Statement * stmt;
		sql::ResultSet * res;
		sql::PreparedStatement * pstmt;

		driver = get_driver_instance();
		con = driver->connect("tcp://192.168.0.43:3306", "program", "a");

		con->setSchema("speed_test");

	    ofstream myfile;
    	myfile.open("copy_c1p1_c2_c3.csv");
		string command;
   		int i=32768,j;
    	int range = 32768;
   		int sum;

 		while(i <= 1048576){
      	sum=0;
   		command = "SELECT c1p1, c2, c3 FROM speedtest_1048576 Limit " + to_string(i);

    for(int j=0; j<5; j++){
		pstmt = con->prepareStatement(command);
		res = pstmt->executeQuery();
		int i = 0;
		*T1_size = res->rowsCount();
		*T1 = (Table1Type*) malloc(sizeof(Table1Type) * *T1_size);
		timer2.start();
		while (res->next())
		{
			T1[0][i].c1p1 = res->getInt("c1p1");
			T1[0][i].c2 = res->getInt("c2");
			T1[0][i].c3 = res->getInt("c3");
			//T1[0][i].c4 = res->getInt("c4");
			i++;
		}

		sum += timer2.elapsedMicroseconds();  
		delete res;
		delete pstmt;
		}
		i+=range;
		myfile << sum/5 << endl;
		

  	  	}

	}
	catch (sql::SQLException &e)
	{

		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << ")" << endl;
		cout << "\n" << EXIT_FAILURE;
	}
}


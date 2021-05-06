#include <stdio.h>
#include <time.h>
#include <fstream>

using namespace std;

int main()
{

    ofstream myfile;
    int i;
    srand(time(NULL));

    myfile.open("speed_262144.csv");
    myfile << "c1p1" << "," << "c2" << "," << "c3" << "," << "fk_s" << "," << "fk_xs" << "," << endl;
    for(i=0; i<262144; i++)
    myfile << i << "," << rand()%101<< "," << rand()%10001<< "," << rand()%131072<< "," << rand()%65536 << endl;

    myfile.close();
    myfile.open("speed_131072.csv");
    myfile << "c1p1" << "," << "c2" << "," << "c3" << "," << "c4" << "," << "fk_xs" << endl;
    for(i=0; i<65536; i++)
    myfile << i << "," << rand()%101<< "," << rand()%10001<< "," << rand()%20001<< "," << i << endl;
    for(i=65536; i<131072; i++)
    myfile << i << "," << rand()%101<< "," << rand()%10001<< "," << rand()%20001<< "," <<  rand()%65536 << endl;

    myfile.close();
    myfile.open("speed_65536.csv");
    myfile << "c1p1" << "," << "c2" << "," << "c3" << "," << "c4" <<  endl;
    for(i=0; i<65536; i++)
    myfile << i << "," << rand()%101<< "," << rand()%1001<< "," << rand()%10001 << endl;

    myfile.close();

    return 0;
}

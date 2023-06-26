#include<iostream>
#include"kvstore.h"
#include<fstream>
#include<sys/time.h>
using namespace std;

const int length = 20000;
const int testNum = 10000;
const string filename = "./com.csv";

int main()
{
    srand(time(NULL));
    struct timeval timeStart, timeEnd;
    KVStore kv = KVStore("./testdata");
    ofstream csvfile(filename, ios::app);
    kv.reset();
    csvfile << "pertime" <<endl;
    for(int i = 0; i < testNum; i++)
    {
        gettimeofday(&timeStart, NULL);
        kv.put((uint64_t)rand(), string(length, 's'));
        gettimeofday(&timeEnd, NULL);
        double Time = 1000 * (double)((timeEnd.tv_sec - timeStart.tv_sec ) + (double)(timeEnd.tv_usec -timeStart.tv_usec)/1000000);
        csvfile << Time << endl;
    }


}

#include<iostream>
#include"kvstore.h"
#include<sys/time.h>
#include<fstream>
using namespace std;

const int testnum = 80000;
const int delnum = 0.2 * testnum;
const int length = 12000;
const string filename = "./normal.csv";

int main()
{
    KVStore kv = KVStore("./testdata");
    kv.reset();
    struct timeval timeStart, timeEnd;
    ofstream csvfile(filename, ios::app);
    csvfile << "Put" << "," << "Del" << "," << "Get" << endl;
    for(int cc = 0; cc < 20; cc++)
    {
    	gettimeofday(&timeStart, NULL);
    	for(int i = 0; i < testnum; i++)
    	{
       	    kv.put((uint64_t)rand(), string(length, 's'));
    	}
        gettimeofday(&timeEnd, NULL);
        double Put = (double)((timeEnd.tv_sec - timeStart.tv_sec ) + (double)(timeEnd.tv_usec -timeStart.tv_usec)/1000000)/testnum;

        gettimeofday(&timeStart, NULL);
    	for(int i = 0; i < delnum; i++)
    	{
       	    kv.del((uint64_t)rand());
    	}
        gettimeofday(&timeEnd, NULL);
        double Del = (double)((timeEnd.tv_sec - timeStart.tv_sec ) + (double)(timeEnd.tv_usec -timeStart.tv_usec)/1000000)/delnum;

        gettimeofday(&timeStart, NULL);
    	for(int i = 0; i < testnum; i++)
    	{
       	    kv.get((uint64_t)rand());
    	}
        gettimeofday(&timeEnd, NULL);
        double Get = (double)((timeEnd.tv_sec - timeStart.tv_sec ) + (double)(timeEnd.tv_usec -timeStart.tv_usec)/1000000)/testnum;
        csvfile << Put << "," << Del << "," << Get << endl;
        kv.reset();
    }
    csvfile.close();
    return 0;
}

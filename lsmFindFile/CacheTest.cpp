#include<iostream>
#include"kvstore.h"
#include<fstream>
#include<sys/time.h>
using namespace std;

const int skipSize = 2097152;

const int insertsize = 15000;
const uint64_t smalltest = 100;
const uint64_t middletest = 5000;
const uint64_t largetest = 30000;
const string filename = "./Cache.csv";

int main()
{
    srand(time(NULL));
    struct timeval timeStart, timeEnd;
    KVStore store = KVStore("./testdata");
    ofstream csvCache(filename, ios::app);
    store.reset();
    for(int i = 0; i < 100; i++)
    {
        store.reset();
        for(int i = 0; i < smalltest; i++)
        {
            store.put((uint64_t)rand(), string(insertsize, 's'));
        }
        gettimeofday(&timeStart, NULL);
        for(int i = 0; i < 10000; i++)
        {
            store.get((uint64_t)rand());
        }
        gettimeofday(&timeEnd, NULL);
        double small = (double)((timeEnd.tv_sec - timeStart.tv_sec ) + (double)(timeEnd.tv_usec -timeStart.tv_usec)/1000000)/10000;
    
        store.reset();
        for(int i = 0; i < middletest; i++)
        {
            store.put((uint64_t)rand(), string(insertsize, 's'));
        }
        gettimeofday(&timeStart, NULL);
        for(int i = 0; i < 10000; i++)
        {
            store.get((uint64_t)rand());
        }
        gettimeofday(&timeEnd, NULL);
        double middle = (double)((timeEnd.tv_sec - timeStart.tv_sec ) + (double)(timeEnd.tv_usec -timeStart.tv_usec)/1000000)/10000;
    
        store.reset();
        for(int i = 0; i < largetest; i++)
        {
            store.put((uint64_t)rand(), string(insertsize, 's'));
        }
        gettimeofday(&timeStart, NULL);
        for(int i = 0; i < 10000; i++)
        {
            store.get((uint64_t)rand());
        }
        gettimeofday(&timeEnd, NULL);
        double large = (double)((timeEnd.tv_sec - timeStart.tv_sec ) + (double)(timeEnd.tv_usec -timeStart.tv_usec)/1000000)/10000;
    
        ifstream file(filename);
		bool isEmpty = file.peek() == ifstream::traits_type::eof();
		file.close();

        if(isEmpty)
		{
			csvCache << "small,middle,large" << endl;
		}

        csvCache << small << "," << middle << "," << large << endl;
    }
    csvCache.close();
    return 0;
    
}

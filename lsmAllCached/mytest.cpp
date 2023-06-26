#include<iostream>
#include"kvstore.h"
#include<fstream>
#include<sys/time.h>
using namespace std;

const uint32_t addTestNum = 4000;
const uint32_t searchNum = addTestNum;
const uint32_t delNum = 0.2 * addTestNum;
const string filename = "datamiddle.csv";

int main()
{
	srand(0);
	struct timeval timeStart, timeEnd, timeSystemStart; 
	KVStore store = KVStore("./data");
	ofstream csvFile(filename, ios::app);
	for(int i = 0; i < 100; i++)
	{
		store.reset();
		gettimeofday(&timeStart, NULL);
		for (int i = 0; i < addTestNum; ++i) {
			store.put(rand()%(int)(1.25 * addTestNum), std::string(12000, 's'));
		}
		gettimeofday(&timeEnd, NULL);
		double putTime = (double)((timeEnd.tv_sec - timeStart.tv_sec ) + (double)(timeEnd.tv_usec -timeStart.tv_usec)/1000000)/addTestNum;  
		//store.reset();

		gettimeofday(&timeStart, NULL);
		for(int i = 0; i < delNum; i++)
		{
			store.del(rand()%(int)(1.25 * addTestNum));
		}
		gettimeofday(&timeEnd, NULL);
		double deleteTime =  (double)((timeEnd.tv_sec - timeStart.tv_sec ) + (double)(timeEnd.tv_usec -timeStart.tv_usec)/1000000)/delNum;
		//store.reset();

		gettimeofday(&timeStart, NULL);	
		for(int i = 0; i < searchNum; i++)
		{
			store.get(rand()%(int)(1.25 * addTestNum));
		}
		gettimeofday(&timeEnd, NULL);
		double getTime =  (double)((timeEnd.tv_sec - timeStart.tv_sec ) + (double)(timeEnd.tv_usec -timeStart.tv_usec)/1000000)/searchNum;

		ifstream file(filename);
		bool isEmpty = file.peek() == ifstream::traits_type::eof();
		file.close();

		if(isEmpty)
		{
			csvFile << "put,del,get" << endl;
		}

		csvFile << putTime << "," << deleteTime << "," << getTime << endl;
	}
	//store.reset();
	csvFile.close();
    return 0;
}

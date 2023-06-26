#include<iostream>
#include"kvstore.h"
#include<fstream>
using namespace std;

int main()
{
	srand(0);
	KVStore store = KVStore("./data");
	store.reset();
	for (int i = 0; i < 1024 * 16; ++i) {
			store.put(i, std::string(i+1, 's'));
	}

	cout << "hereeeeee======\n";

    return 0;
}


LINK.o = $(LINK.cc)
CXXFLAGS = -std=c++14 -Wall -g

all: correctness persistence

correctness: kvstore.o correctness.o SST.o

persistence: kvstore.o persistence.o SST.o

mytest: kvstore.o mytest.o SST.o

CacheTest: kvstore.o CacheTest.o SST.o

comTest: kvstore.o comTest.o SST.o

clean:
	-rm -f correctness persistence mytest CacheTest comTest *.o

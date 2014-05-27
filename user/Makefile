CXX ?= g++
CPPFLAGS += -Wall -Werror -pedantic -std=c++11

mkfs.aufs: mkfs.o
	$(CXX) $(LDFLAGS) mkfs.o -o mkfs.aufs

mkfs.o: mkfs.cpp aufs.hpp block.hpp
	$(CXX) $(CPPFLAGS) -c mkfs.cpp -o mkfs.o

clean:
	rm -rf *.o mkfs.aufs

.PHONY: clean

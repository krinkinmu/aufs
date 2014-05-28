CXX ?= clang++
CPPFLAGS += -Wall -Werror -pedantic -std=c++11

mkfs.aufs: mkfs.o block.o
	$(CXX) $(LDFLAGS) mkfs.o block.o -o mkfs.aufs

mkfs.o: mkfs.cpp aufs.hpp block.hpp
	$(CXX) $(CPPFLAGS) -c mkfs.cpp -o mkfs.o

block.o: block.cpp aufs.hpp block.hpp
	$(CXX) $(CPPFLAGS) -c block.cpp -o block.o

clean:
	rm -rf *.o mkfs.aufs

.PHONY: clean

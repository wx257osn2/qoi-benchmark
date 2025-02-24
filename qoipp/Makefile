CXXFLAGS=-std=c++2a -O3 -march=native -mtune=native -Wall -Wextra -pedantic-errors -I qoipp/include

all: libqoipp.a

libqoipp.a: driver.o qoipp.o
	$(AR) rcs $@ $^

driver.o: driver.cpp qoipp/include/qoipp.hpp
	$(CXX) -c $(CXXFLAGS) -o$@ $<

qoipp.o: qoipp/source/qoipp.cpp qoipp/include/qoipp.hpp
	$(CXX) -c $(CXXFLAGS) -o$@ $<

clean:
	$(RM) -f libqoipp.a driver.o qoipp.o

.PHONY: all clean

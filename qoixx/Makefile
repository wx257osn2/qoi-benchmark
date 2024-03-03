CXXFLAGS=-std=c++2a -O3 -march=native -mtune=native -Wall -Wextra -pedantic-errors -I qoixx/include

all: libqoixx.a libqoixx_nosimd.a

libqoixx.a: driver.o
	$(AR) rcs $@ $<

driver.o: driver.cpp qoixx/include/qoixx.hpp
	$(CXX) -c $(CXXFLAGS) -DQOIXX_PREFIX=qoixx -o$@ $<

libqoixx_nosimd.a: driver_nosimd.o
	$(AR) rcs $@ $<

driver_nosimd.o: driver.cpp qoixx/include/qoixx.hpp
	$(CXX) -c $(CXXFLAGS) -DQOIXX_PREFIX=qoixx_nosimd -DQOIXX_NO_SIMD -DQOIXX_DECODE_WITH_TABLES=0 -o$@ $<

clean:
	$(RM) -f libqoixx.a driver.o libqoixx_nosimd.a driver_nosimd.o

.PHONY: all clean

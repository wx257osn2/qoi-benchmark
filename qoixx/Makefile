libqoixx.a: driver.o
	$(AR) rcs $@ $<

driver.o: driver.cpp qoixx/include/qoixx.hpp
	$(CXX) -c -std=c++2a -O3 -march=native -mtune=native -Wall -Wextra -pedantic-errors -I qoixx/include -o$@ $<

clean:
	$(RM) -f libqoixx.a driver.o

.PHONY: clean

libqoi_fu_cxx.a: qoi-fu.cpp.o QOI.cpp.o
	$(AR) rcs $@ $^

CXX_FLAGS:=-std=c++2a -O3 -march=native -mtune=native -Wall -Wextra -pedantic-errors -DNDEBUG

qoi-fu.cpp.o: qoi-fu.cpp
	$(CXX) -c $(CXX_FLAGS) -I qoi-fu/transpiled -o$@ $<

QOI.cpp.o: qoi-fu/transpiled/QOI.cpp
	$(CXX) -c $(CXX_FLAGS) -o$@ $<

clean:
	$(RM) -f libqoi_fu_cxx.a qoi-fu.cpp.o QOI.cpp.o

.PHONY: clean

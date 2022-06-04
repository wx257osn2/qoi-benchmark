qoibench: bin/qoibench

clean:
	$(RM) -f bin/qoibench lib/*.a include/qoi.h

.PHONY: qoibench clean

bin/qoibench: src/qoibench.cpp include/implementations.hpp include/implementation_macro.hpp lib/libqoi.a lib/libqoixx.a lib/libqoi_rust.a lib/librapid_qoi.a
	$(CXX) -std=c++2a -Wall -Wextra -pedantic-errors -O3 -march=native -mtune=native -I .dependencies/stb -I include -o$@ $< -Llib -lqoi -lqoixx -lqoi_rust -lrapid_qoi -pthread -ldl

all: libqoi libqoixx libqoi_rust benchmark

benchmark: benchmark/bin/benchmark

libqoi: qoi/libqoi.a

libqoixx: qoixx/libqoixx.a

clean:
	cd qoi && make clean && cd ..
	cd qoixx && make clean && cd ..
	cd benchmark && make clean && cd ..

install_libraries: libqoi libqoixx
	cp qoi/qoi/qoi.h benchmark/include
	cp qoi/libqoi.a benchmark/lib
	cp qoixx/libqoixx.a benchmark/lib

.PHONY: all benchmark libqoi libqoixx install_libraries clean

qoi/libqoi.a: qoi/qoi/qoi.h qoi/qoi.c qoi/Makefile
	cd qoi && make

qoixx/libqoixx.a: qoixx/qoixx/include/qoixx.hpp qoixx/driver.cpp qoixx/Makefile
	cd qoixx && make

benchmark/bin/benchmark: install_libraries benchmark/src/qoibench.cpp benchmark/Makefile
	cd benchmark && make

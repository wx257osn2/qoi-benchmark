all: libqoi libqoixx libqoi_rust librapid_qoi libqoi_fu_cxx benchmark

benchmark: benchmark/bin/benchmark

libqoi: qoi/libqoi.a

libqoixx: qoixx/libqoixx.a

libqoixx_nosimd: qoixx/libqoixx_nosimd.a

libqoi_rust: qoi-rust/target/release/libqoi_rust.a

librapid_qoi: rapid-qoi/target/release/librapid_qoi.a

libqoi_fu_cxx: qoi-fu/libqoi_fu_cxx.a

clean:
	cd qoi && make clean && cd ..
	cd qoixx && make clean && cd ..
	cd qoi-rust && cargo clean && cd ..
	cd rapid-qoi && cargo clean && cd ..
	cd qoi-fu && make clean && cd ..
	cd benchmark && make clean && cd ..

install_libraries: libqoi libqoixx libqoixx_nosimd libqoi_rust librapid_qoi libqoi_fu_cxx
	cp qoi/qoi/qoi.h benchmark/include
	cp qoi/libqoi.a benchmark/lib
	cp qoixx/libqoixx.a benchmark/lib
	cp qoixx/libqoixx_nosimd.a benchmark/lib
	cp qoi-rust/target/release/libqoi_rust.a benchmark/lib
	cp rapid-qoi/target/release/librapid_qoi.a benchmark/lib
	cp qoi-fu/libqoi_fu_cxx.a benchmark/lib

.PHONY: all benchmark libqoi libqoixx libqoixx_nosimd libqoi_rust librapid_qoi libqoi_fu_cxx install_libraries clean

qoi/libqoi.a: qoi/qoi/qoi.h qoi/qoi.c qoi/Makefile
	cd qoi && make

qoixx/libqoixx.a: qoixx/qoixx/include/qoixx.hpp qoixx/driver.cpp qoixx/Makefile
	cd qoixx && make libqoixx.a

qoixx/libqoixx_nosimd.a: qoixx/qoixx/include/qoixx.hpp qoixx/driver.cpp qoixx/Makefile
	cd qoixx && make libqoixx_nosimd.a

qoi-rust/target/release/libqoi_rust.a: qoi-rust/src/lib.rs qoi-rust/Cargo.toml
	cd qoi-rust && cargo build --release

rapid-qoi/target/release/librapid_qoi.a: rapid-qoi/src/lib.rs rapid-qoi/Cargo.toml rapid-qoi/rapid-qoi/src/decode.rs rapid-qoi/rapid-qoi/src/encode.rs rapid-qoi/rapid-qoi/src/lib.rs
	cd rapid-qoi && cargo build --release

qoi-fu/libqoi_fu_cxx.a: qoi-fu/qoi-fu/transpiled/QOI.hpp qoi-fu/qoi-fu/transpiled/QOI.cpp qoi-fu/qoi-fu.cpp qoi-fu/Makefile
	cd qoi-fu && make

benchmark/bin/benchmark: install_libraries benchmark/src/qoibench.cpp benchmark/Makefile
	cd benchmark && make

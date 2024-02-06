qoibench: bin/qoibench

stat_analyzer: bin/stat_analyzer

clean:
	$(RM) -f bin/qoibench bin/stat_analyzer lib/*.a include/qoi.h

.PHONY: qoibench stat_analyzer clean

bin/qoibench: src/qoibench.cpp include/implementations.hpp include/implementation_macro.hpp lib/libqoi.a lib/libqoixx.a lib/libqoixx_nosimd.a lib/libqoi_rust.a lib/librapid_qoi.a lib/libqoi_fu_cxx.a
	$(CXX) -std=c++2a -Wall -Wextra -pedantic-errors -O3 -march=native -mtune=native -I .dependencies/stb -I .dependencies/nlohmann_json/include -I include -o$@ $< -Llib -lqoi -lqoixx -lqoixx_nosimd -lqoi_rust -lrapid_qoi -lqoi_fu_cxx -pthread -ldl

bin/stat_analyzer: src/stat_analyzer.cpp include/implementations.hpp include/implementation_macro.hpp
	$(CXX) -std=c++2a -Wall -Wextra -pedantic-errors -O3 -march=native -mtune=native -I .dependencies/nlohmann_json/include -I include -o$@ $<

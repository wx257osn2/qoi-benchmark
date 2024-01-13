# QOI Benchmark

Cross-language benchmark for [QOI](https://github.com/phoboslab/qoi) implementations

## Implementations

- [`qoi`](https://github.com/phoboslab/qoi)
- [`qoixx`](https://github.com/wx257osn2/qoixx)
- [`qoi-rust`](https://github.com/aldanor/qoi-rust)
- [`rapid-qoi`](https://github.com/zakarumych/rapid-qoi)
- [`qoi-fu`](https://github.com/pfusik/qoi-fu)
    - `qoi-fu_cxx` : implementation transpiled to C++

## Usage

- Use Docker
    - ```console
      host$ curl https://qoiformat.org/benchmark/qoi_benchmark_suite.tar | tar x
      host$ ./docker/build.bash
      host$ ./docker/bash
      docker$ make
      docker$ benchmark/bin/qoibench 20 images
      ```
- On Host
    - Prerequisites:
        - `gcc` for `qoi`
        - `g++` supporting `-std=c++2a` for `qoixx` and `qoi-fu_cxx`
        - `cargo` and Rust toolchains for `qoi-rust` and `rapid-qoi`
        - `make`
    - ```console
      $ make
      $ benchmark/bin/qoibench 20 /path/to/image/directory
      ```

## How to add my implementation?

You can add your own QOI implementation in this project.
If you want it, you need to implement C interface as same as [original implementaion](https://github.com/phoboslab/qoi).
More concretely, you should implement below functions:

```c
void* your_own_qoi_encode(const void* data, const qoi_desc* desc, int* out_len);
void* your_own_qoi_decode(const void* data, int size, qoi_desc* desc, int channels); // it's OK that the last argument doesn't work. In this project channels is same as desc.channels.

void your_own_qoi_free(void* ptr); // provide appropriate deallocation method corresponding to allocation method you used in encode/decode
```

Then, create static library (`libyour_own_qoi.a`) with your implementations.
Finally, add new entry about your implementation in [`benchmark/include/implementations.hpp`](https://github.com/wx257osn2/qoi-benchmark/blob/master/benchmark/include/implementations.hpp).

The example is [here](https://github.com/wx257osn2/qoi-benchmark/commit/34fb816c61020cd67eece14df26ccf0f3997e11a).

## License

MIT

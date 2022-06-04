# QOI Benchmark

Cross-language benchmark for [QOI](https://github.com/phoboslab/qoi) implementations

## Implementations

- [`qoi`](https://github.com/phoboslab/qoi)
- [`qoixx`](https://github.com/wx257osn2/qoixx)
- [`qoi-rust`](https://github.com/aldanor/qoi-rust)
- [`rapid-qoi`](https://github.com/zakarumych/rapid-qoi)

## Usage

```console
$ make
$ benchmark/bin/benchmark 20 /path/to/image/directory
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
Finally, add some codes that use your static library in `benchmark/src/qoibench.cpp` .

The example is [here](https://github.com/wx257osn2/qoi-benchmark/commit/bf6f803de2ac63cd0105cdb66bf458dbb0d6d25a).

## License

MIT

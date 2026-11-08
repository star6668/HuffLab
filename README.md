# HuffLab

A lossless compression library written in C++20, implementing Huffman coding and Run-Length Encoding (RLE).

I started this project from a Huffman coding assignment and expanded it into a standalone compression library with file I/O, a command-line interface, and basic performance benchmarks.

I'm currently working on improving its performance and exploring low-level C++ optimization.

## Features

- Huffman and RLE compression
- Binary file compression and decompression
- In-memory C++ API
- Command-line interface
- Compression benchmarks
- CMake integration and automated tests

## Getting started

**Requirements:** C++20 compiler and CMake 3.20+

### Build

```bash
git clone https://github.com/star6668/HuffLab.git
cd HuffLab

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
```

### Usage

Compress a file:

```bash
./build/hufflab compress input.txt output.huf
```

Decompress it:

```bash
./build/hufflab decompress output.huf restored.txt
```

Run a benchmark:

```bash
./build/hufflab benchmark input.txt
```

You can also use `--method rle` or `--method combined` to choose a different compression method.

*On Windows, the executable path may differ depending on your CMake generator.*

## Using HuffLab as a library

HuffLab can also be integrated into another C++ project.

```cpp
#include <hufflab/codec.hpp>

int main() {
    hufflab::Bytes data{'H', 'e', 'l', 'l', 'o'};

    auto compressed = hufflab::compress(data);
    auto restored = hufflab::decompress(compressed);

    return restored == data ? 0 : 1;
}
```

The library can be installed and imported using CMake's `find_package()`.

See `examples/consumer` for a complete integration example.

## Performance

HuffLab includes a benchmark command to compare Huffman, RLE, and combined compression.

The benchmark reports:

- Compressed file size
- Compression ratio
- Compression and decompression time
- Round-trip verification

These measurements are currently intended for comparison and experimentation rather than formal performance claims.

## Project structure

```text
include/hufflab/   Public headers
src/               Compression engine and CLI
tests/             Unit and integration tests
examples/          Library usage examples
```

## Current limitations

HuffLab is still experimental. Files are currently processed in memory, the archive format is not yet stable, and there is no checksum or authentication. It should not be used to decompress untrusted archives.

## Next steps

- [ ] Profile and optimize Huffman encoding/decoding
- [ ] Add streaming compression
- [ ] Reduce memory allocations
- [ ] Explore multithreaded compression
- [ ] Improve archive validation and robustness

## License

License information will be added once the distribution terms are finalized.
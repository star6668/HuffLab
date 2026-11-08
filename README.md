# HuffLab — A reusable C++20 lossless compression library

HuffLab is a small **C++ library and CLI** for lossless binary compression.
It implements Huffman coding, RLE, and RLE followed by Huffman, with an
independent package consumer to verify that another project can link to it.

> Current status: an **experimental library**, not a production-hardened
> compression format. Its decoder is not intended for untrusted archives.

## Features

- In-memory `hufflab::compress` / `hufflab::decompress` API.
- File-based `hufflab::compress_file` / `hufflab::decompress_file` API.
- Reusable CMake package: `find_package(HuffLab CONFIG REQUIRED)` and
  `target_link_libraries(my_app PRIVATE HuffLab::hufflab)`.
- Linux and Windows CI: original Huffman tests, archive round-trip tests,
  file API tests and an independently compiled consumer program.
- Existing `hufflab` CLI and original lab report preserved.

## Build and test

Requires CMake >= 3.20 and a C++20 compiler.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --build-config Release --output-on-failure
```

## CLI usage

```sh
./build/hufflab compress sample.txt sample.huf
./build/hufflab decompress sample.huf restored.txt
./build/hufflab compress sample.txt sample-rle.huf --method rle
./build/hufflab benchmark sample.txt
./build/hufflab analyze sample.txt
```

With a multi-config generator on Windows, the executable may be
`build\\Release\\hufflab.exe`.

## Use HuffLab from *another* C++ project

### 1. Install the library into a local directory

```sh
cmake --install build --config Release --prefix "$PWD/install"
```

### 2. Configure and compile an independent consumer

```sh
cmake -S examples/consumer -B consumer-build -DCMAKE_PREFIX_PATH="$PWD/install"
cmake --build consumer-build --config Release
ctest --test-dir consumer-build --build-config Release --output-on-failure
```

In your own `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyApp LANGUAGES CXX)
find_package(HuffLab 0.2 CONFIG REQUIRED)
add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE HuffLab::hufflab)
```

And in `main.cpp`:

```cpp
#include <hufflab/codec.hpp>
#include <hufflab/file.hpp>

int main() {
    const hufflab::Bytes input{'H', 'e', 'l', 'l', 'o'};
    const auto packed = hufflab::compress(input);
    const auto restored = hufflab::decompress(packed);
    if (restored != input) return 1;

    auto stats = hufflab::compress_file("source.txt", "source.huf");
    (void)stats;
    hufflab::decompress_file("source.huf", "restored.txt");
}
```

The last two calls require an existing `source.txt`; the code above
illustrates both APIs.

### Public headers

- `<hufflab/codec.hpp>`: binary in-memory API and `Method` enum.
- `<hufflab/file.hpp>`: file compression/decompression and `FileResult`.

The headers `huffman.hpp` and `bitstream.hpp` remain in the source tree for
coursework and tests, but are intentionally **not installed** as a public API.
The library currently processes whole files in memory: there is no streaming
interface or concurrency yet.

## Archive format: HFL1

Each file begins with `HFL1` (4 bytes), a method identifier (1 byte), and
original size (unsigned little-endian 64-bit, 8 bytes). Huffman methods add a
2-byte count of unique symbols and sorted pairs `(symbol:1 byte,
frequency:8 bytes)` before a MSB-first packed bitstream. Plain RLE uses
`(run length:1 byte, value:1 byte)` pairs after the common header.
The format is specific to HuffLab, not ZIP/gzip compatible.

## Reliability and limitations

- Archive headers can overwhelm savings on small inputs; some files grow.
- `benchmark` times a single run; figures are for exploration, not rigorous
  reproducible performance claims.
- File helpers load all bytes into memory and overwrite the destination on
  successful output. They are not atomic or crash-safe operations.
- Structure and length validation exist, but the format has no checksum or
  authentication. **Do not decompress untrusted files or use this for critical backups.**
- The format is not yet stable; future versions may change it.

## Source layout

- `include/hufflab/codec.hpp`, `include/hufflab/file.hpp`: installed public API.
- `src/codec.cpp`, `src/bitstream.cpp`, `src/file.cpp`: codec and I/O layer.
- `src/huffman.cpp`, `include/hufflab/huffman.hpp`: original lab algorithms.
- `src/main.cpp`: CLI; `examples/lab_report.cpp`: original lab report.
- `examples/consumer`: standalone consumer using `find_package`.
- `tests/`: original lab tests, codec tests and file API tests.
- `.github/workflows/ci.yml`: Linux and Windows build/test/install/consumer check.

## Before making the repository public

The Huffman starter and original tests came from coursework. Verify the
course's publication rules before publishing adapted educational materials.
Do not choose a code license that covers files you lack the rights to license.

#pragma once

#include <cstdint>
#include <vector>

namespace hufflab {

using Bytes = std::vector<std::uint8_t>;

enum class Method : std::uint8_t {
  huffman = 1,
  rle = 2,
  rle_huffman = 3
};

// Self-contained .huf bytes (HFL1 format). May grow on incompressible inputs.
Bytes compress(const Bytes& input, Method method = Method::huffman);
Bytes decompress(const Bytes& archive);

} // namespace hufflab

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace hufflab {

// Bits are packed most-significant bit first. Final byte is zero-padded.
class BitWriter {
public:
  void write_bit(bool bit);
  std::vector<std::uint8_t> finish();

private:
  std::vector<std::uint8_t> bytes_;
  std::uint8_t current_ = 0;
  unsigned filled_ = 0;
};

class BitReader {
public:
  explicit BitReader(std::span<const std::uint8_t> bytes) : bytes_(bytes) {}
  bool read_bit();  // Throws if the input is truncated.
  std::size_t bits_read() const { return position_; }

private:
  std::span<const std::uint8_t> bytes_;
  std::size_t position_ = 0;
};

} // namespace hufflab

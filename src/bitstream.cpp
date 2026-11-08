#include "hufflab/bitstream.hpp"

#include <stdexcept>
#include <utility>

namespace hufflab {

void BitWriter::write_bit(bool bit) {
  current_ = static_cast<std::uint8_t>((current_ << 1) | (bit ? 1u : 0u));
  if (++filled_ == 8) {
    bytes_.push_back(current_);
    current_ = 0;
    filled_ = 0;
  }
}

std::vector<std::uint8_t> BitWriter::finish() {
  if (filled_ != 0) {
    bytes_.push_back(static_cast<std::uint8_t>(current_ << (8 - filled_)));
    current_ = 0;
    filled_ = 0;
  }
  return std::move(bytes_);
}

bool BitReader::read_bit() {
  if (position_ / 8 >= bytes_.size())
    throw std::invalid_argument("truncated Huffman bitstream");
  auto const byte = bytes_[position_ / 8];
  auto const bit = ((byte >> (7 - position_ % 8)) & 1u) != 0;
  ++position_;
  return bit;
}

} // namespace hufflab

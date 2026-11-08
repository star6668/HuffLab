#include "hufflab/codec.hpp"

#include "hufflab/bitstream.hpp"
#include "hufflab/huffman.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace hufflab {
namespace {

// v1 file format: "HFL1", method (1 byte), original size (u64 little-endian),
// [number of symbols (u16), repeated (symbol u8, frequency u64)] for Huffman,
// then packed Huffman bits or RLE (count, value) pairs.
constexpr std::array<std::uint8_t, 4> magic{'H','F','L','1'};

void write_u64(Bytes& out, std::uint64_t n) {
  for (unsigned i = 0; i < 8; ++i) {
    out.push_back(static_cast<std::uint8_t>(n & 0xffu));
    n >>= 8;
  }
}

void write_u16(Bytes& out, std::uint16_t n) {
  out.push_back(static_cast<std::uint8_t>(n & 0xffu));
  out.push_back(static_cast<std::uint8_t>(n >> 8));
}

class HeaderReader {
public:
  explicit HeaderReader(const Bytes& data) : data_(data) {}

  std::uint8_t byte() {
    if (offset_ >= data_.size()) throw std::invalid_argument("truncated .huf header");
    return data_[offset_++];
  }

  std::uint64_t u64() {
    std::uint64_t result = 0;
    for (unsigned i = 0; i < 8; ++i)
      result |= static_cast<std::uint64_t>(byte()) << (8 * i);
    return result;
  }

  std::uint16_t u16() {
    const auto low = byte();
    const auto high = byte();
    return static_cast<std::uint16_t>(low | (static_cast<unsigned>(high) << 8));
  }

  std::span<const std::uint8_t> remaining() const {
    return {data_.data() + offset_, data_.size() - offset_};
  }

private:
  const Bytes& data_;
  std::size_t offset_ = 0;
};

using Code = std::vector<std::uint8_t>;
using Codes = std::array<Code, 256>;

void collect_codes(const std::vector<node>& tree, int rank, Code& prefix,
                   Codes& codes) {
  const auto& n = tree[static_cast<std::size_t>(rank)];
  if (n.left == -1 && n.right == -1) {
    codes[n.symbol] = prefix.empty() ? Code{0} : prefix;
    return;
  }
  prefix.push_back(0);
  collect_codes(tree, n.left, prefix, codes);
  prefix.back() = 1;
  collect_codes(tree, n.right, prefix, codes);
  prefix.pop_back();
}

Bytes encode_bits(const Bytes& input, const std::vector<node>& tree, int root) {
  if (input.empty()) return {};
  Codes codes{};
  Code prefix;
  collect_codes(tree, root, prefix, codes);
  BitWriter writer;
  for (auto b : input)
    for (auto bit : codes[b]) writer.write_bit(bit != 0);
  return writer.finish();
}

std::size_t checked_size(std::uint64_t value) {
  if (value > std::numeric_limits<std::size_t>::max())
    throw std::invalid_argument("file size cannot fit in memory");
  return static_cast<std::size_t>(value);
}

// The frequency table must describe exactly the message being decoded. It also
// lets us verify the payload length before allocating the output buffer.
std::size_t expected_payload_bytes(const counts& frequencies,
                                   const std::vector<node>& tree, int root) {
  const auto code_lengths = measure_codes(tree, root);
  std::size_t bits = 0;
  for (std::size_t i = 0; i < frequencies.size(); ++i) {
    if (code_lengths[i] != 0 &&
        frequencies[i] > (std::numeric_limits<std::size_t>::max() - bits) / code_lengths[i])
      throw std::invalid_argument("Huffman bit count overflow");
    bits += frequencies[i] * code_lengths[i];
  }
  return bits / 8 + (bits % 8 != 0);
}

Bytes decode_bits(std::span<const std::uint8_t> payload,
                  const counts& frequencies, std::size_t decoded_size) {
  std::vector<node> tree;
  const int root = build_tree(tree, frequencies);
  if (payload.size() != expected_payload_bytes(frequencies, tree, root))
    throw std::invalid_argument("incorrect Huffman payload length");
  if (decoded_size == 0) return {};
  if (root < 0) throw std::invalid_argument("missing Huffman tree");

  BitReader reader(payload);
  Bytes result;
  result.reserve(decoded_size);
  for (std::size_t i = 0; i < decoded_size; ++i) {
    int rank = root;
    const auto& root_node = tree[static_cast<std::size_t>(rank)];
    if (root_node.left == -1 && root_node.right == -1) {
      if (reader.read_bit()) throw std::invalid_argument("invalid single-symbol encoding");
    } else {
      while (tree[static_cast<std::size_t>(rank)].left != -1) {
        const bool bit = reader.read_bit();
        const auto& n = tree[static_cast<std::size_t>(rank)];
        rank = bit ? n.right : n.left;
        if (rank < 0) throw std::invalid_argument("invalid Huffman tree traversal");
      }
    }
    result.push_back(tree[static_cast<std::size_t>(rank)].symbol);
  }
  // Zero padding bits are required by our encoder.
  while (reader.bits_read() % 8 != 0)
    if (reader.read_bit()) throw std::invalid_argument("nonzero padding bits");

  if (count_bytes(result) != frequencies)
    throw std::invalid_argument("Huffman frequencies do not match header");
  return result;
}

Bytes checked_rle_decode(std::span<const std::uint8_t> payload,
                         std::size_t expected_size) {
  if (payload.size() % 2 != 0)
    throw std::invalid_argument("RLE payload has incomplete pair");
  std::size_t count = 0;
  for (std::size_t i = 0; i < payload.size(); i += 2) {
    if (payload[i] == 0 || payload[i] > expected_size - count)
      throw std::invalid_argument("invalid RLE count or size");
    count += payload[i];
  }
  if (count != expected_size)
    throw std::invalid_argument("RLE size does not match header");
  return decode_rle(Bytes(payload.begin(), payload.end()));
}

void add_frequencies(Bytes& archive, const counts& frequencies) {
  write_u16(archive, static_cast<std::uint16_t>(count_distinct(frequencies)));
  for (std::size_t i = 0; i < frequencies.size(); ++i) {
    if (frequencies[i] == 0) continue;
    archive.push_back(static_cast<std::uint8_t>(i));
    write_u64(archive, frequencies[i]);
  }
}

counts read_frequencies(HeaderReader& reader, std::size_t& total) {
  counts frequencies{};
  const auto n = reader.u16();
  if (n > 256) throw std::invalid_argument("invalid symbol count");
  int last = -1;
  total = 0;
  for (unsigned i = 0; i < n; ++i) {
    const int symbol = reader.byte();
    const auto weight = reader.u64();
    if (symbol <= last || weight == 0)
      throw std::invalid_argument("invalid Huffman frequency table");
    last = symbol;
    const auto size = checked_size(weight);
    if (size > std::numeric_limits<std::size_t>::max() - total)
      throw std::invalid_argument("frequency sum overflow");
    frequencies[static_cast<std::size_t>(symbol)] = size;
    total += size;
  }
  return frequencies;
}

} // namespace

Bytes compress(const Bytes& input, Method method) {
  if (method != Method::huffman && method != Method::rle &&
      method != Method::rle_huffman)
    throw std::invalid_argument("unsupported compression method");

  Bytes archive(magic.begin(), magic.end());
  archive.push_back(static_cast<std::uint8_t>(method));
  write_u64(archive, static_cast<std::uint64_t>(input.size()));

  if (method == Method::rle) {
    const auto encoded = encode_rle(input);
    archive.insert(archive.end(), encoded.begin(), encoded.end());
    return archive;
  }

  const Bytes prepared = method == Method::rle_huffman ? encode_rle(input) : input;
  const counts frequencies = count_bytes(prepared);
  add_frequencies(archive, frequencies);
  std::vector<node> tree;
  const int root = build_tree(tree, frequencies);
  const auto payload = encode_bits(prepared, tree, root);
  archive.insert(archive.end(), payload.begin(), payload.end());
  return archive;
}

Bytes decompress(const Bytes& archive) {
  HeaderReader reader(archive);
  for (auto expected : magic)
    if (reader.byte() != expected) throw std::invalid_argument("not a HFL1 archive");

  const auto method = static_cast<Method>(reader.byte());
  if (method != Method::huffman && method != Method::rle &&
      method != Method::rle_huffman)
    throw std::invalid_argument("unsupported .huf method");

  const auto original_size = checked_size(reader.u64());
  if (method == Method::rle)
    return checked_rle_decode(reader.remaining(), original_size);

  std::size_t prepared_size = 0;
  const auto frequencies = read_frequencies(reader, prepared_size);
  if (method == Method::huffman && prepared_size != original_size)
    throw std::invalid_argument("Huffman frequencies disagree with original size");
  if (method == Method::rle_huffman &&
      (original_size == 0 ? prepared_size != 0 :
        (original_size > std::numeric_limits<std::size_t>::max() / 2 ||
         prepared_size > original_size * 2)))
    throw std::invalid_argument("invalid combined-method size");

  auto prepared = decode_bits(reader.remaining(), frequencies, prepared_size);
  if (method == Method::rle_huffman)
    return checked_rle_decode(prepared, original_size);
  return prepared;
}

} // namespace hufflab

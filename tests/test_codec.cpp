#include "hufflab/bitstream.hpp"
#include "hufflab/codec.hpp"
#include "hufflab/huffman.hpp"

#include <array>
#include <cstdint>
#include <exception>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using hufflab::Bytes;

namespace {
int failures = 0;

void check(bool condition, const std::string& name) {
  if (!condition) {
    ++failures;
    std::cerr << "FAILED: " << name << '\n';
  }
}

void roundtrip(const Bytes& data, const std::string& name) {
  for (auto method : {hufflab::Method::huffman, hufflab::Method::rle,
                      hufflab::Method::rle_huffman}) {
    const auto compressed = hufflab::compress(data, method);
    check(hufflab::decompress(compressed) == data, name);
  }
}

template <typename F>
void must_reject(F action, const std::string& name) {
  bool rejected = false;
  try { action(); } catch (const std::exception&) { rejected = true; }
  check(rejected, name);
}
}

int main() {
  roundtrip({}, "empty file");
  roundtrip({42}, "single byte");
  roundtrip(Bytes(1024, 7), "repeated byte");
  roundtrip(Bytes{0, 255, 0, 255, 1, 1, 2, 3}, "binary bytes");
  roundtrip(make_sierpinski(512), "Sierpinski image");
  roundtrip(make_xor_texture(), "XOR image");
  Bytes every_byte;
  for (int b = 0; b < 256; ++b) every_byte.push_back(static_cast<std::uint8_t>(b));
  roundtrip(every_byte, "all 256 symbols");

  std::mt19937 rng(1234);
  for (auto n : {1u, 2u, 7u, 8u, 9u, 31u, 257u, 1024u, 4097u}) {
    Bytes random_data(n);
    for (auto& b : random_data) b = static_cast<std::uint8_t>(rng() & 0xffu);
    roundtrip(random_data, "random bytes");
  }

  auto good = hufflab::compress(Bytes{1, 1, 1, 2, 3, 3});
  auto bad_magic = good;
  bad_magic[0] = 0;
  must_reject([&] { (void)hufflab::decompress(bad_magic); }, "bad magic");
  auto bad_method = good;
  bad_method[4] = 99;
  must_reject([&] { (void)hufflab::decompress(bad_method); }, "bad method");
  auto truncated = good;
  truncated.pop_back();
  must_reject([&] { (void)hufflab::decompress(truncated); }, "truncated payload");
  must_reject([&] { (void)hufflab::decompress(Bytes{}); }, "empty archive");
  auto rle = hufflab::compress(Bytes{4, 4}, hufflab::Method::rle);
  rle.pop_back();
  must_reject([&] { (void)hufflab::decompress(rle); }, "truncated RLE pair");

  hufflab::BitWriter writer;
  for (bool bit : {true, false, true, true, false, false, true, false, true})
    writer.write_bit(bit);
  check(writer.finish() == Bytes({0xb2, 0x80}), "MSB-first packing");

  if (failures == 0) std::cout << "codec checks pass\n";
  return failures == 0 ? 0 : 1;
}

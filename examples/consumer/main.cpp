#include <hufflab/codec.hpp>

#include <cstdint>
#include <iostream>
#include <vector>

int main() {
  const hufflab::Bytes original{1, 1, 1, 1, 2, 3, 3, 3};
  const auto archive = hufflab::compress(original, hufflab::Method::huffman);
  const auto restored = hufflab::decompress(archive);

  if (original != restored) return 1;
  std::cout << "Library imported successfully: " << original.size()
            << " original bytes, " << archive.size() << " archive bytes\n";
  return 0;
}

#include "hufflab/codec.hpp"
#include "hufflab/file.hpp"
#include "hufflab/huffman.hpp"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using hufflab::Bytes;
using Clock = std::chrono::steady_clock;

Bytes read_file_or_throw(const std::string& path) {
  std::ifstream file(path, std::ios::binary);
  if (!file) throw std::runtime_error("cannot open input: " + path);
  Bytes bytes{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
  if (file.bad()) throw std::runtime_error("error while reading: " + path);
  return bytes;
}

hufflab::Method parse_method(const std::string& name) {
  if (name == "huffman") return hufflab::Method::huffman;
  if (name == "rle") return hufflab::Method::rle;
  if (name == "combined") return hufflab::Method::rle_huffman;
  throw std::invalid_argument("unknown method: " + name);
}

void usage() {
  std::cout << "HuffLab - lossless file compression (HFL1)\n\n"
            << "Usage:\n"
            << "  hufflab compress <input> <output.huf> [--method huffman|rle|combined]\n"
            << "  hufflab decompress <input.huf> <output>\n"
            << "  hufflab analyze <input>\n"
            << "  hufflab benchmark <input>\n";
}

void analyze(const Bytes& data) {
  const auto frequencies = count_bytes(data);
  const auto entropy = compute_entropy(frequencies);
  std::vector<node> tree;
  const int root = build_tree(tree, frequencies);
  const auto lengths = measure_codes(tree, root);
  const std::size_t bits = count_bits(frequencies, lengths);

  std::cout << "Input bytes:               " << data.size() << '\n'
            << "Distinct symbols:          " << count_distinct(frequencies) << '\n'
            << "Shannon entropy:           " << std::fixed << std::setprecision(3)
            << entropy << " bits/symbol\n"
            << "Huffman payload estimate:  " << (bits / 8 + (bits % 8 != 0))
            << " bytes (without header)\n";
}

void benchmark(const Bytes& data) {
  std::cout << "Method             Archive bytes   Saved (%)   Compress ms   Decode ms   Verify\n";
  std::cout << "-------------------------------------------------------------------------------\n";
  for (const auto [name, method] :
       {std::pair{"huffman", hufflab::Method::huffman},
        std::pair{"rle", hufflab::Method::rle},
        std::pair{"combined", hufflab::Method::rle_huffman}}) {
    const auto t0 = Clock::now();
    auto compressed = hufflab::compress(data, method);
    const auto t1 = Clock::now();
    auto restored = hufflab::decompress(compressed);
    const auto t2 = Clock::now();
    double saved = data.empty() ? 0.0 :
        100.0 * (1.0 - static_cast<double>(compressed.size()) / data.size());
    auto ms = [](auto a, auto b) {
      return std::chrono::duration<double, std::milli>(b-a).count();
    };
    std::cout << std::left << std::setw(19) << name
              << std::right << std::setw(13) << compressed.size()
              << std::setw(12) << std::fixed << std::setprecision(1) << saved
              << std::setw(14) << std::setprecision(3) << ms(t0, t1)
              << std::setw(12) << ms(t1, t2)
              << std::setw(9) << (restored == data ? "OK" : "FAIL") << '\n';
    if (restored != data) throw std::runtime_error("benchmark round-trip failed");
  }
}

} // namespace

int main(int argc, char* argv[]) {
  try {
    if (argc < 2 || std::string(argv[1]) == "--help" ||
        std::string(argv[1]) == "-h") {
      usage();
      return argc < 2 ? 1 : 0;
    }
    const std::string command = argv[1];
    if (command == "compress" && (argc == 4 || argc == 6)) {
      if (argc == 6 && std::string(argv[4]) != "--method")
        throw std::invalid_argument("expected --method");
      const auto method = argc == 6 ? parse_method(argv[5]) : hufflab::Method::huffman;
      const auto result = hufflab::compress_file(argv[2], argv[3], method);
      std::cout << "Compressed " << result.input_bytes << " -> " << result.output_bytes
                << " bytes (includes metadata)\n";
      return 0;
    }
    if (command == "decompress" && argc == 4) {
      const auto result = hufflab::decompress_file(argv[2], argv[3]);
      std::cout << "Decompressed " << result.input_bytes << " -> "
                << result.output_bytes << " bytes\n";
      return 0;
    }
    if ((command == "analyze" || command == "benchmark") && argc == 3) {
      const auto bytes = read_file_or_throw(argv[2]);
      if (command == "analyze") analyze(bytes);
      else benchmark(bytes);
      return 0;
    }
    usage();
    return 1;
  } catch (const std::exception& error) {
    std::cerr << "HuffLab error: " << error.what() << '\n';
    return 1;
  }
}

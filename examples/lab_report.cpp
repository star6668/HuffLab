#include "hufflab/huffman.hpp"

#include <cstdio>
#include <numeric>
#include <string>
#include <vector>

namespace
{
  // ----------------------------------------------------------------- part 3

  void report(std::string const& nom, std::vector<std::uint8_t> const& data,
             std::size_t octets_sur_disque)
  {
    if (data.empty()) {
      std::printf("%-20s  (empty source)\n", nom.c_str());
      return;
    }

    auto const c = count_bytes(data);
    std::vector<node> tree;
    int const root = build_tree(tree, c);
    auto const l = measure_codes(tree, root);

    std::size_t const symbols = data.size();
    std::size_t const distinct = count_distinct(c);
    double const entropy = compute_entropy(c);
    std::size_t const bits = count_bits(c, l);
    double const average = static_cast<double>(bits) / symbols;
    double const gap = average - entropy;
    std::size_t const bytes = bits / 8 + (bits % 8 != 0);

    char gain[24] = "-";
    if (octets_sur_disque != 0) {
      double const pct = 100.0 * (1.0 - static_cast<double>(bytes) /
                                         octets_sur_disque);
      std::snprintf(gain, sizeof(gain), "%.1f%%", pct);
    }
    std::printf("%-20s %9zu %6zu %8.3f %8.3f %8.3f %10zu %7s\n",
                nom.c_str(), symbols, distinct, entropy, average,
                gap, bytes, gain);
  }
}

int main(int argc, char** argv)
{
  std::printf("%-20s %9s %6s %8s %8s %8s %10s %7s\n",
              "source", "symbols", "dist.", "entropy", "Huffman", "gap",
              "bytes", "gain");
  std::printf("%s\n", std::string(90, '-').c_str());

  auto const triangle = make_sierpinski(512);
  auto const texture = make_xor_texture();

  report("sierpinski 512", triangle, 0);
  report("sierpinski RLE", encode_rle(triangle), triangle.size());
  report("xor texture", texture, 0);
  report("xor texture RLE", encode_rle(texture), texture.size());

  // An image given as an argument: one from an earlier lab, for instance.
  for (int a = 1; a < argc; ++a)
  {
    auto const raw = read_file(argv[a]);
    auto const pixels = read_pixels(argv[a]);

    std::string nom = argv[a];
    nom = nom.substr(nom.find_last_of("/\\") + 1);

    // An image is read pixel by pixel; anything else -- a text, a source
    // file -- is taken as the bytes it holds.
    report(nom, pixels.empty() ? raw : pixels, raw.size());
  }

  // Question 4.1 asks for this number to be recovered without counting.
  std::printf("\nThe triangle holds %zu points out of %zu.\n",
              count_bytes(make_sierpinski(512))[0], std::size_t{512} * 512);

  return 0;
}

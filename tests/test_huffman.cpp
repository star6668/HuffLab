#include "hufflab/huffman.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

static int failures = 0;
static void check(bool condition, char const* what) {
  if (!condition) { std::cerr << "FAILED: " << what << '\n'; ++failures; }
}

namespace {
  bool near(double a, double b, double eps = 1e-10) {
    return std::abs(a - b) < eps;
  }

  // Encoded message size, excluding metadata needed to store the Huffman tree.
  std::size_t encoded_bits(std::vector<std::uint8_t> const& data) {
    auto const c = count_bytes(data);
    std::vector<node> tree;
    int const root = build_tree(tree, c);
    return count_bits(c, measure_codes(tree, root));
  }

  void check_huffman_properties(std::vector<std::uint8_t> const& data) {
    auto const c = count_bytes(data);
    if (data.empty()) return; // no average to compare with entropy
    std::vector<node> tree;
    int const root = build_tree(tree, c);
    auto const l = measure_codes(tree, root);

    double const average = static_cast<double>(count_bits(c, l)) / data.size();
    double const h = compute_entropy(c);
    check(average + 1e-10 >= h, "Shannon lower bound");
    check(average <= h + 1.0 + 1e-10, "Huffman upper bound");

    std::size_t const distinct = count_distinct(c);
    if (distinct >= 2) {
      double kraft = 0.0;
      for (std::size_t i = 0; i < c.size(); ++i) {
        if (c[i] != 0) kraft += std::exp2(-static_cast<double>(l[i]));
      }
      check(near(kraft, 1.0), "Kraft equality for a full tree");
    }

    for (std::size_t i = 0; i < c.size(); ++i)
      for (std::size_t j = 0; j < c.size(); ++j)
        if (c[j] > 0 && c[i] > c[j])
          check(l[i] <= l[j], "More frequent symbols are no longer");
  }
}

int main() {
  // Part 1: a concrete source, hand-computed entropy, empty case.
  {
    std::vector<std::uint8_t> const data = {1, 1, 1, 2};
    auto const c = count_bytes(data);
    check(c[1] == 3, "byte 1 occurs three times");
    check(c[2] == 1, "byte 2 only once");
    check(c[0] == 0, "byte 0 never");
    check(count_distinct(c) == 2, "two distinct values");
    check(near(compute_entropy(count_bytes({1, 1, 2, 2})), 1.0),
          "two equally likely symbols have entropy 1");
    check(near(compute_entropy(count_bytes({1, 2, 3, 4})), 2.0),
          "four equally likely symbols have entropy 2");
    check(near(compute_entropy(count_bytes({7, 7, 7})), 0.0),
          "a constant source has entropy 0");

    auto const empty = count_bytes({});
    check(std::all_of(empty.begin(), empty.end(),
                      [](std::size_t n){ return n == 0; }),
          "empty source has zero counts");
    check(count_distinct(empty) == 0, "empty source has no distinct symbol");
    check(near(compute_entropy(empty), 0.0), "empty entropy is 0 by convention");
  }

  // Part 2: empty, single symbol, 2n-1 nodes and conservation of weight.
  {
    std::vector<node> tree;
    int root = build_tree(tree, count_bytes({}));
    check(root == -1 && tree.empty(), "empty tree");

    auto const single = count_bytes({42, 42, 42});
    root = build_tree(tree, single);
    check(root >= 0 && tree.size() == 1, "one leaf for one symbol");
    check(tree[static_cast<std::size_t>(root)].weight == 3,
          "single root holds total weight");

    auto const four = count_bytes({0,0,0,0,1,1,2,3});
    root = build_tree(tree, four);
    check(root >= 0 && tree.size() == 7, "4 leaves produce 7 nodes");
    check(tree[static_cast<std::size_t>(root)].weight == 8,
          "root weight equals source length");
  }

  // Part 3: exact expected code lengths and 14 coded bits.
  {
    auto const c = count_bytes({0,0,0,0,1,1,2,3});
    std::vector<node> tree;
    int const root = build_tree(tree, c);
    auto const l = measure_codes(tree, root);
    check(l[0] == 1 && l[1] == 2 && l[2] == 3 && l[3] == 3,
          "lengths for 4,2,1,1 are 1,2,3,3");
    check(count_bits(c, l) == 14, "4,2,1,1 take 14 bits");
    check(encoded_bits({}) == 0, "empty message takes zero bits");
    check(encoded_bits({42,42,42}) == 3, "one-symbol message uses 1 bit/symbol");
    check_huffman_properties({0,0,0,0,1,1,2,3});
  }

  // Part 4: different optimal tie resolutions still yield 23 bits.
  {
    std::string const word = "abracadabra";
    std::vector<std::uint8_t> const data(word.begin(), word.end());
    auto const c = count_bytes(data);
    check(c['a'] == 5 && c['b'] == 2 && c['r'] == 2
          && c['c'] == 1 && c['d'] == 1, "abracadabra frequencies");
    check(count_distinct(c) == 5, "abracadabra has 5 values");
    check(encoded_bits(data) == 23, "abracadabra takes 23 bits");
    check_huffman_properties(data);
  }

  // Part 5: Sierpinski counts, XOR uniformity, Huffman bounds.
  {
    auto const small = make_sierpinski(256);
    auto const big = make_sierpinski(512);
    check(small.size() == 256u*256u && count_bytes(small)[0] == 6561,
          "256x256 triangle contains 3^8 zero-valued points");
    check(big.size() == 512u*512u && count_bytes(big)[0] == 19683,
          "512x512 triangle contains 3^9 zero-valued points");
    check(encoded_bits(big) == big.size(), "Sierpinski takes 1 bit/pixel");
    check(near(compute_entropy(count_bytes(big)), 0.38461841422736587,
               0.00001), "Sierpinski entropy approx 0.385");
    check_huffman_properties(big);
    check_huffman_properties(small);

    auto const xor_pixels = make_xor_texture();
    auto const c = count_bytes(xor_pixels);
    check(count_distinct(c) == 256, "XOR uses all 256 byte values");
    check(std::all_of(c.begin(), c.end(), [](std::size_t n){ return n == 256; }),
          "XOR has uniform byte distribution");
    check(near(compute_entropy(c), 8.0), "XOR entropy equals 8 bits");
    check(encoded_bits(xor_pixels) == 8 * xor_pixels.size(),
          "XOR Huffman is 8 bits/pixel");
    check_huffman_properties(xor_pixels);
  }

  // Part 6: empty, 255 limit, round-trip, image experiments.
  {
    check(encode_rle({}).empty(), "RLE encodes empty source as empty");
    check(decode_rle({}).empty(), "RLE decodes empty source as empty");
    check(encode_rle({9,9,9,2,2,3}) ==
          std::vector<std::uint8_t>({3,9,2,2,1,3}), "RLE pair encoding");
    std::vector<std::uint8_t> const run300(300, 7);
    check(encode_rle(run300) ==
          std::vector<std::uint8_t>({255,7,45,7}),
          "RLE splits a 300-byte run into 255+45");
    check(decode_rle(encode_rle(run300)) == run300,
          "long run survives round-trip");
    auto const triangle = make_sierpinski(512);
    auto const texture = make_xor_texture();
    auto const triangle_rle = encode_rle(triangle);
    auto const texture_rle = encode_rle(texture);
    check(decode_rle(triangle_rle) == triangle, "Sierpinski RLE round-trip");
    check(decode_rle(texture_rle) == texture, "XOR RLE round-trip");
    check(triangle_rle.size() == 39884, "Sierpinski RLE uses 39884 bytes");
    check(texture_rle.size() == 131070, "XOR RLE uses 131070 bytes");
    check_huffman_properties(triangle_rle);
    check_huffman_properties(texture_rle);
    bool rejected = false;
    try { (void)decode_rle({1}); }
    catch (std::invalid_argument const&) { rejected = true; }
    check(rejected, "invalid odd-sized RLE data is rejected");
  }

  if (failures == 0) std::cout << "all checks pass\n";
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

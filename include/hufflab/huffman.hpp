#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

/// How many times each possible byte occurs.
using counts = std::array<std::size_t, 256>;

/// The code length of each byte, in bits. Zero for a byte that is absent.
using lengths = std::array<std::size_t, 256>;

/// A node of the Huffman tree. Children are ranks in the vector holding the
/// tree, not pointers: the vector may be reallocated, a rank stays valid, an
/// address does not.
///
/// A leaf has no children: both its ranks are -1.
struct node
{
  int left = -1, right = -1;
  std::uint8_t symbol = 0;
  std::size_t weight = 0;

  bool operator==(node const&) const = default;
};

// ----------------------------------------------------------------- part 1

/// How many times each byte occurs.
counts count_bytes(std::vector<std::uint8_t> const& data);

/// How many different bytes occur at least once.
std::size_t count_distinct(counts const& c);

/// The entropy of the source, in bits per symbol. It is the bound no coding
/// working symbol by symbol can cross.
double compute_entropy(counts const& c);

// ----------------------------------------------------------------- part 2

/// Builds the tree into `tree` and returns the rank of its root, or -1 when
/// the counts are all zero.
int build_tree(std::vector<node>& tree, counts const& c);

// ----------------------------------------------------------------- part 3

/// The code length of each symbol, obtained by walking down the tree.
lengths measure_codes(std::vector<node> const& tree, int root);

/// The total number of bits of the coded message.
std::size_t count_bits(counts const& c, lengths const& l);

// ----------------------------------------------------------------- part 5

/// The two images from lab 3, rebuilt here to serve as test subjects.

/// The Sierpinski triangle on a square of side `side`: a point belongs to it
/// when `x & y` is zero. `side` must be a power of two.
std::vector<std::uint8_t> make_sierpinski(int side);

/// The exclusive-or texture on a square of side 256.
std::vector<std::uint8_t> make_xor_texture();

// ----------------------------------------------------------------- part 6

/// The run-length encoding of `data`: pairs (count, value), the count never
/// above 255. A run longer than that is cut into several pairs.
std::vector<std::uint8_t> encode_rle(std::vector<std::uint8_t> const& data);

/// The inverse of `encode_rle`.
std::vector<std::uint8_t> decode_rle(std::vector<std::uint8_t> const& data);

// ------------------------------------------------------- given: les fichiers

/// The bytes of a file, as they are.
std::vector<std::uint8_t> read_file(std::string const& path);

/// The pixel values of a PGM or a PPM in text form. Returns an empty vector
/// when the file does not read.
std::vector<std::uint8_t> read_pixels(std::string const& path);

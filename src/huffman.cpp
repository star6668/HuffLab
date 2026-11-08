#include "hufflab/huffman.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iterator>
#include <numeric>
#include <stdexcept>

// ----------------------------------------------------------------- part 1

counts count_bytes(std::vector<std::uint8_t> const& data)
{
  counts c{};
  std::for_each(data.begin(), data.end(), [&c](std::uint8_t byte) {
    ++c[byte];
  });
  return c;
}

std::size_t count_distinct(counts const& c)
{
  return static_cast<std::size_t>(std::count_if(
      c.begin(), c.end(), [](std::size_t n) { return n != 0; }));
}

double compute_entropy(counts const& c)
{
  std::size_t const total =
      std::accumulate(c.begin(), c.end(), std::size_t{0});
  if (total == 0) return 0.0;

  return std::accumulate(c.begin(), c.end(), 0.0,
                         [total](double entropy, std::size_t n) {
    if (n == 0) return entropy; // log2(0) is undefined
    double const p = static_cast<double>(n) / total;
    return entropy - p * std::log2(p);
  });
}

// ----------------------------------------------------------------- part 2

int build_tree(std::vector<node>& tree, counts const& c)
{
  tree.clear();
  std::vector<int> roots;
  for (std::size_t symbol = 0; symbol < c.size(); ++symbol) {
    if (c[symbol] == 0) continue;
    int const rank = static_cast<int>(tree.size());
    node leaf{};
    leaf.weight = c[symbol];
    leaf.symbol = static_cast<std::uint8_t>(symbol);
    leaf.left = -1;
    leaf.right = -1;

    tree.push_back(leaf);
    roots.push_back(rank);
  }

  while (roots.size() > 1) {
    // Sort ranks, not nodes: indices remain valid when tree is reallocated.
    std::sort(roots.begin(), roots.end(), [&tree](int a, int b) {
      auto const wa = tree[static_cast<std::size_t>(a)].weight;
      auto const wb = tree[static_cast<std::size_t>(b)].weight;
      if (wa != wb) return wa < wb;
      return a < b; // deterministic ties
    });
    int const left = roots[0];
    int const right = roots[1];
    roots.erase(roots.begin(), roots.begin() + 2);
    std::size_t const weight = tree[static_cast<std::size_t>(left)].weight
                             + tree[static_cast<std::size_t>(right)].weight;
    int const parent = static_cast<int>(tree.size());
    node parent_node{};
    parent_node.weight = weight;
    parent_node.symbol = 0;
    parent_node.left = left;
    parent_node.right = right;

    tree.push_back(parent_node);
   
    roots.push_back(parent);
  }

  return roots.empty() ? -1 : roots.front();
}

// ----------------------------------------------------------------- part 3

namespace
{
  void walk(std::vector<node> const& tree, int rank, std::size_t depth,
               lengths& out)
  {
    if (rank < 0) return;

    node const& n = tree[static_cast<std::size_t>(rank)];

    if (n.left < 0 && n.right < 0)
    {
      // A tree reduced to one leaf gives a depth of zero. The shortest code
      // is still one bit: a symbol is not coded on nothing.
      out[n.symbol] = (depth == 0) ? 1 : depth;
      return;
    }

    walk(tree, n.left, depth + 1, out);
    walk(tree, n.right, depth + 1, out);
  }
}

lengths measure_codes(std::vector<node> const& tree, int root)
{
  lengths out{};
  walk(tree, root, 0, out);
  return out;
}

std::size_t count_bits(counts const& c, lengths const& l)
{
  return std::inner_product(c.begin(), c.end(), l.begin(), std::size_t{0});
}

// ----------------------------------------------------------------- part 5

std::vector<std::uint8_t> make_sierpinski(int side)
{
  if (side <= 0) return {};
  std::vector<std::uint8_t> pixels(static_cast<std::size_t>(side) * side);
  std::size_t index = 0;
  std::generate(pixels.begin(), pixels.end(), [side, &index]() {
    std::size_t const x = index % static_cast<std::size_t>(side);
    std::size_t const y = index / static_cast<std::size_t>(side);
    ++index;
    return static_cast<std::uint8_t>((x & y) == 0 ? 0 : 255);
  });
  return pixels;
}

std::vector<std::uint8_t> make_xor_texture()
{
  std::vector<std::uint8_t> pixels(256u * 256u);
  std::size_t index = 0;
  std::generate(pixels.begin(), pixels.end(), [&index]() {
    std::size_t const x = index % 256u;
    std::size_t const y = index / 256u;
    ++index;
    return static_cast<std::uint8_t>(x ^ y);
  });
  return pixels;
}

// ----------------------------------------------------------------- part 6

std::vector<std::uint8_t> encode_rle(std::vector<std::uint8_t> const& data)
{
  std::vector<std::uint8_t> encoded;
  auto it = data.begin();
  while (it != data.end()) {
    std::uint8_t const value = *it;
    auto const next = std::find_if(it, data.end(), [value](std::uint8_t byte) {
      return byte != value;
    });
    std::size_t run = static_cast<std::size_t>(next - it);
    while (run > 0) {
      std::size_t const count = std::min(run, std::size_t{255});
      encoded.push_back(static_cast<std::uint8_t>(count));
      encoded.push_back(value);
      run -= count;
    }
    it = next;
  }
  return encoded;
}

std::vector<std::uint8_t> decode_rle(std::vector<std::uint8_t> const& data)
{
  if (data.size() % 2 != 0)
    throw std::invalid_argument("RLE data must contain (count, value) pairs");
  std::vector<std::uint8_t> decoded;
  for (std::size_t i = 0; i < data.size(); i += 2) {
    std::uint8_t const count = data[i];
    if (count == 0)
      throw std::invalid_argument("RLE count must be positive");
    decoded.insert(decoded.end(), count, data[i + 1]);
  }
  return decoded;
}

// ------------------------------------------------------- given: les fichiers

std::vector<std::uint8_t> read_file(std::string const& path)
{
  std::ifstream f(path, std::ios::binary);
  if (!f) return {};

  return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
}

std::vector<std::uint8_t> read_pixels(std::string const& path)
{
  std::ifstream f(path);
  if (!f) return {};

  std::string magique;
  int w = 0, h = 0, maxval = 0;

  f >> magique >> w >> h >> maxval;
  if ((magique != "P2" && magique != "P3") || w <= 0 || h <= 0) return {};

  int const par_pixel = (magique == "P3") ? 3 : 1;

  std::vector<std::uint8_t> v;
  v.reserve(static_cast<std::size_t>(w) * h * par_pixel);

  for (int i = 0; i < w * h * par_pixel; ++i)
  {
    int x = 0;
    if (!(f >> x)) return {};
    v.push_back(static_cast<std::uint8_t>(x));
  }

  return v;
}

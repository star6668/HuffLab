#include "hufflab/file.hpp"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>

namespace hufflab {
namespace {

void check_paths(const std::filesystem::path& source,
                 const std::filesystem::path& destination) {
  // weakly_canonical works even when the output file does not exist yet.
  if (std::filesystem::weakly_canonical(source) ==
      std::filesystem::weakly_canonical(destination))
    throw std::invalid_argument("input and output must be different paths");
  // Detect hard links as well as symlinks, if both names already exist.
  std::error_code ec;
  if (std::filesystem::equivalent(source, destination, ec) && !ec)
    throw std::invalid_argument("input and output refer to the same file");
}

Bytes read_all(const std::filesystem::path& path) {
  std::ifstream stream(path, std::ios::binary);
  if (!stream) throw std::runtime_error("cannot open input: " + path.string());
  Bytes data{std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
  if (stream.bad()) throw std::runtime_error("error while reading: " + path.string());
  return data;
}

void write_all(const std::filesystem::path& path, const Bytes& data) {
  if (data.size() > static_cast<std::uint64_t>(std::numeric_limits<std::streamsize>::max()))
    throw std::runtime_error("output too large for a single write");
  std::ofstream stream(path, std::ios::binary | std::ios::trunc);
  if (!stream) throw std::runtime_error("cannot create output: " + path.string());
  if (!data.empty())
    stream.write(reinterpret_cast<const char*>(data.data()),
                 static_cast<std::streamsize>(data.size()));
  stream.close();
  if (!stream) throw std::runtime_error("error while writing: " + path.string());
}

} // namespace

FileResult compress_file(const std::filesystem::path& input,
                         const std::filesystem::path& output, Method method) {
  check_paths(input, output);
  const auto source = read_all(input);
  const auto archive = compress(source, method);
  write_all(output, archive);
  return {static_cast<std::uint64_t>(source.size()),
          static_cast<std::uint64_t>(archive.size())};
}

FileResult decompress_file(const std::filesystem::path& input,
                           const std::filesystem::path& output) {
  check_paths(input, output);
  const auto archive = read_all(input);
  const auto restored = decompress(archive);
  write_all(output, restored);
  return {static_cast<std::uint64_t>(archive.size()),
          static_cast<std::uint64_t>(restored.size())};
}

} // namespace hufflab

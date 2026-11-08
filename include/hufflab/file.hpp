#pragma once

#include "hufflab/codec.hpp"

#include <cstdint>
#include <filesystem>

namespace hufflab {

// File helpers for the HFL1 format. These convenience functions load the whole
// file into memory; a future streaming interface will remove that limitation.
struct FileResult {
  std::uint64_t input_bytes = 0;
  std::uint64_t output_bytes = 0;
};

// Throws std::exception on I/O or archive errors. Source and destination must
// refer to different files. An existing destination is overwritten on success.
FileResult compress_file(const std::filesystem::path& input,
                         const std::filesystem::path& output,
                         Method method = Method::huffman);
FileResult decompress_file(const std::filesystem::path& input,
                           const std::filesystem::path& output);

} // namespace hufflab

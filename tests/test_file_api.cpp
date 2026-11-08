#include "hufflab/file.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>

int main() {
  namespace fs = std::filesystem;
  const auto base = fs::temp_directory_path() / "hufflab_file_api_test";
  fs::create_directories(base);
  const auto input = base / "source.bin";
  const auto archive = base / "source.huf";
  const auto restored = base / "restored.bin";
  const std::vector<std::uint8_t> data{0, 0, 17, 17, 17, 255, 42, 42};
  {
    std::ofstream stream(input, std::ios::binary | std::ios::trunc);
    stream.write(reinterpret_cast<const char*>(data.data()),
                 static_cast<std::streamsize>(data.size()));
  }
  const auto result = hufflab::compress_file(input, archive);
  if (result.input_bytes != data.size() || result.output_bytes == 0) return 1;
  const auto undo = hufflab::decompress_file(archive, restored);
  if (undo.input_bytes != result.output_bytes || undo.output_bytes != data.size()) return 1;
  std::ifstream in(restored, std::ios::binary);
  const std::vector<std::uint8_t> restored_data{
      std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
  if (restored_data != data) return 1;
  bool rejected = false;
  try { (void)hufflab::compress_file(input, input); }
  catch (const std::invalid_argument&) { rejected = true; }
  if (!rejected) return 1;
  // A corrupt archive must be rejected before an output file is opened.
  {
    std::ofstream broken(archive, std::ios::binary | std::ios::trunc);
    broken << "bad";
  }
  bool corrupt_rejected = false;
  try { (void)hufflab::decompress_file(archive, restored); }
  catch (const std::invalid_argument&) { corrupt_rejected = true; }
  if (!corrupt_rejected) return 1;
  fs::remove_all(base);
  std::cout << "File API checks pass\n";
  return 0;
}

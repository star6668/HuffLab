# CMake generated Testfile for 
# Source directory: /home/celine/HuffLab-library
# Build directory: /home/celine/HuffLab-library/build-release
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[original_huffman_lab]=] "/home/celine/HuffLab-library/build-release/test_huffman")
set_tests_properties([=[original_huffman_lab]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/celine/HuffLab-library/CMakeLists.txt;43;add_test;/home/celine/HuffLab-library/CMakeLists.txt;0;")
add_test([=[codec_roundtrip]=] "/home/celine/HuffLab-library/build-release/test_codec")
set_tests_properties([=[codec_roundtrip]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/celine/HuffLab-library/CMakeLists.txt;47;add_test;/home/celine/HuffLab-library/CMakeLists.txt;0;")
add_test([=[public_file_api]=] "/home/celine/HuffLab-library/build-release/test_file_api")
set_tests_properties([=[public_file_api]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/celine/HuffLab-library/CMakeLists.txt;51;add_test;/home/celine/HuffLab-library/CMakeLists.txt;0;")

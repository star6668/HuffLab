cmake_minimum_required(VERSION 3.20)
project(HuffLab VERSION 0.2.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(HUFFLAB_BUILD_CLI "Build the command-line tool" ON)
option(HUFFLAB_BUILD_EXAMPLES "Build the original lab report" ON)

add_library(hufflab_core STATIC
    src/huffman.cpp
    src/bitstream.cpp
    src/codec.cpp
    src/file.cpp
)
add_library(HuffLab::hufflab ALIAS hufflab_core)
set_target_properties(hufflab_core PROPERTIES EXPORT_NAME hufflab)
target_compile_features(hufflab_core PUBLIC cxx_std_20)
target_include_directories(hufflab_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
if(MSVC)
    target_compile_options(hufflab_core PRIVATE /W4)
else()
    target_compile_options(hufflab_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(HUFFLAB_BUILD_CLI)
    add_executable(hufflab src/main.cpp)
    target_link_libraries(hufflab PRIVATE HuffLab::hufflab)
endif()

if(HUFFLAB_BUILD_EXAMPLES)
    add_executable(hufflab_lab_report examples/lab_report.cpp)
    target_link_libraries(hufflab_lab_report PRIVATE HuffLab::hufflab)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(test_huffman tests/test_huffman.cpp)
    target_link_libraries(test_huffman PRIVATE HuffLab::hufflab)
    add_test(NAME original_huffman_lab COMMAND test_huffman)

    add_executable(test_codec tests/test_codec.cpp)
    target_link_libraries(test_codec PRIVATE HuffLab::hufflab)
    add_test(NAME codec_roundtrip COMMAND test_codec)

    add_executable(test_file_api tests/test_file_api.cpp)
    target_link_libraries(test_file_api PRIVATE HuffLab::hufflab)
    add_test(NAME public_file_api COMMAND test_file_api)
endif()

# Install only the supported public interface: codec.hpp and file.hpp.
# bitstream.hpp / huffman.hpp remain available to the original lab and tests
# within the source tree but are not part of the installed API.
install(TARGETS hufflab_core EXPORT HuffLabTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES
        include/hufflab/codec.hpp
        include/hufflab/file.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hufflab)

set(HUFFLAB_CMAKE_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/HuffLab")
configure_package_config_file(
    cmake/HuffLabConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/HuffLabConfig.cmake"
    INSTALL_DESTINATION "${HUFFLAB_CMAKE_DIR}"
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/HuffLabConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(EXPORT HuffLabTargets
        FILE HuffLabTargets.cmake
        NAMESPACE HuffLab::
        DESTINATION "${HUFFLAB_CMAKE_DIR}")
install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/HuffLabConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/HuffLabConfigVersion.cmake"
        DESTINATION "${HUFFLAB_CMAKE_DIR}")

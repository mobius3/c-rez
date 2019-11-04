cmake_minimum_required(VERSION 3.0)
project(c-rez
  VERSION 1.0.1
  LANGUAGES C
  DESCRIPTION "c-rez is a small tool to generate C arrays of data from files."
)

# Honor C_VISIBILITY_PRESET and CXX_VISIBILITY_PRESET
cmake_policy(SET CMP0063 NEW)

add_executable(c-rez src/c-rez.c)
add_executable(c-rez::c-rez ALIAS c-rez)
if (WIN32)
  target_compile_definitions(c-rez PRIVATE -D_CRT_SECURE_NO_WARNINGS)
endif()

install(TARGETS c-rez EXPORT c-rez-targets RUNTIME DESTINATION bin)
include(src/c-rez-add-c-rez.cmake)

# Export c-rez-config
include(CMakePackageConfigHelpers)
write_basic_package_version_file("${CMAKE_BINARY_DIR}/c-rez-config-version.cmake" VERSION "${PROJECT_VERSION}" COMPATIBILITY AnyNewerVersion)
install(EXPORT c-rez-targets FILE c-rez-targets.cmake NAMESPACE c-rez:: DESTINATION lib/cmake/c-rez)
install(FILES
  src/c-rez-config.cmake
  src/c-rez-add-c-rez.cmake
  ${CMAKE_BINARY_DIR}/c-rez-config-version.cmake
  DESTINATION lib/cmake/c-rez)

include(src/c-rez-cpack.cmake)

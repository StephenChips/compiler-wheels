# CMakeList.txt : CMake project for parser-generator, include source and define
# project specific logic here.
#

find_package(fmt CONFIG REQUIRED)

# Add source to this project's executable.
add_library (parser-generator "parser-generator.cpp" "parser-generator.h")
target_link_libraries(parser-generator fmt::fmt)


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET parser-generator PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.

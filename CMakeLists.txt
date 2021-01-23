cmake_minimum_required(VERSION 3.16)
project (nanaide)
include(FetchContent)
set(CMAKE_CXX_STANDARD 17)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()
 
# these libs are not (yet?) available in conan, so we have them as a local dependency
# lets try and use conan as much as posible
FetchContent_Declare(
  nana
  GIT_REPOSITORY https://github.com/cnjinhao/nana
  GIT_TAG        v1.7.4
)

FetchContent_Declare(
  sqlitecpp
  GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git
  GIT_TAG        3.1.1
)
FetchContent_MakeAvailable(sqlitecpp nana)

add_executable(nanaide WIN32 main.cpp)
target_link_libraries(nanaide nana SQLiteCpp sqlite3)

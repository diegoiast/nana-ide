cmake_minimum_required(VERSION 3.16)
project (nanaide)
include(FetchContent)
set(CMAKE_CXX_STANDARD 17)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

################################################################################
# these libs are not (yet?) available in conan, so we have them as a local
# dependency lets try and use conan as much as posible


# Nana UI
# TODO this should enable to build Nana as a shared library, does not work now
# TODO package Nana UI as a conan library
#set(BUILD_SHARED_LIBS ON)
add_compile_definitions(NANA_ENABLE_PNG NANA_ENABLE_JPEG)
if (UNIX)
    add_compile_definitions(USE_LIBJPEG_FROM_OS  USE_LIBPNG_FROM_OS)
endif (UNIX)
FetchContent_Declare(
  nana
  GIT_REPOSITORY https://github.com/cnjinhao/nana
  GIT_TAG        v1.7.4
)
FetchContent_MakeAvailable(nana)
################################################################################


add_executable(nanaide WIN32 main.cpp)
target_link_libraries(nanaide nana ${CONAN_LIBS})

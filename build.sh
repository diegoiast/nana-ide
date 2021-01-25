#/bin/sh

set -x
set -e

export PATH=$PATH:"$HOME/.local/bin/"

# Disable these if you don't have ccache installed
CC="ccache clang"
CXX="ccache clang++"
CMAKE_GENERATOR="Ninja"
#CMAKE_GENERATOR="Unix Makefiles"
BUILD_DIR="cbuild"

rm -fr $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR
conan install ../
cmake .. -G "$CMAKE_GENERATOR"
cd ..
cmake --build $BUILD_DIR --parallel

echo on


set PATH=%PATH%;c:\Program Files\CMake\bin\
set PATH=%PATH%;c:\tdm\gcc\bin\

rem Disable these if you don't have ccache installed
set CC=ccache gcc
set CXX=ccache g++
set CMAKE_GENERATOR="Ninja"
set BUILD_DIR="cbuild"

rmdir /q /s %BUILD_DIR%
mkdir %BUILD_DIR%
cd %BUILD_DIR%
conan install ..
cmake .. -G %CMAKE_GENERATOR% 
cd ..
cmake --build %BUILD_DIR%

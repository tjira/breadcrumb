# minimum cmake requirement
cmake_minimum_required(VERSION 3.18)

# project name
project(Breadcrump)

# set the c++ standard
set(CMAKE_CXX_STANDARD 20)

# set policies and compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set output path
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

# set the debug and release compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -pg -Og -Wall -Wextra -Wl,-rpath,'$ORIGIN' --coverage")
set(CMAKE_CXX_FLAGS_RELEASE "-s -O3 -flto=auto -march=native -Wl,-rpath,'$ORIGIN'")

# set static flags
if(STATIC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
endif()

# include directories
include_directories(include external/include)

# find custom packages
find_library(LIBGMPXX NAMES gmpxx PATHS external/lib NO_DEFAULT_PATH)
find_library(LIBMPFR NAMES mpfr PATHS external/lib NO_DEFAULT_PATH)
find_library(LIBGMP NAMES gmp PATHS external/lib NO_DEFAULT_PATH)

# add executables
add_executable(fractal    src/cpp/fractal.cpp )
add_executable(mersenne   src/cpp/mersenne.cpp)

# link the libraries
target_link_libraries(fractal  ${LIBGMP} ${LIBGMPXX} ${LIBMPFR})
target_link_libraries(mersenne ${LIBGMP} ${LIBGMPXX}           )

# find system packages
find_package(OpenMP REQUIRED)

# link OpenMP
if(OpenMP_CXX_FOUND)
    target_link_libraries(fractal    OpenMP::OpenMP_CXX)
    target_link_libraries(mersenne   OpenMP::OpenMP_CXX)
endif()

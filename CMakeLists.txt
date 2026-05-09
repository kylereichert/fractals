cmake_minimum_required(VERSION 3.20)

project(Fractals)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(
  SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-3.4.4
)
FetchContent_Declare(
  eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG 5.0.1
)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 12.1.0
)

FetchContent_MakeAvailable(SDL3 eigen fmt)

add_executable(Fractals fractals.cpp)

target_link_libraries(Fractals PRIVATE
                        fmt::fmt
                        Eigen3::Eigen
                        SDL3::SDL3
                      )


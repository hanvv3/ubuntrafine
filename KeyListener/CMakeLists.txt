cmake_minimum_required(VERSION 3.10)
project(BrightnessKeyListener)

set(CMAKE_CXX_STANDARD 14)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBEVDEV REQUIRED libevdev)

include_directories(${LIBEVDEV_INCLUDE_DIRS})
link_directories(${LIBEVDEV_LIBRARY_DIRS})

add_executable(BrightnessKeyListener listen_keys.cpp)
target_link_libraries(BrightnessKeyListener ${LIBEVDEV_LIBRARIES})

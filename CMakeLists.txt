cmake_minimum_required(VERSION 3.16)
project(open_installer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBZIP REQUIRED libzip)

include_directories(
    include
    ${CURL_INCLUDE_DIRS}
    ${LIBZIP_INCLUDE_DIRS}
)

set(SOURCES
    src/main.cpp
    src/http.cpp
    src/extractor.cpp
    src/installer.cpp
)

add_executable(installer ${SOURCES})

target_link_libraries(installer
    PRIVATE
    ${CURL_LIBRARIES}
    ${LIBZIP_LIBRARIES}
)

if (WIN32)
    target_link_libraries(installer PRIVATE ws2_32)
endif()

cmake_minimum_required(VERSION 3.13)
project (ok.ru-downloader-t)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-sign-compare")

set(LINKING_LIBRARIES "-lcurl -lwolfssl -lz -lm -lcares -lnghttp2")

add_executable (ok.ru-downloader-t ok.ru-downloader.cpp)
target_link_libraries (ok.ru-downloader-t ${LINKING_LIBRARIES} -static)

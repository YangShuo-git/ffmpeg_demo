cmake_minimum_required (VERSION 3.16)
project(ffmpeg_demo)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")   
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
# C++11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")  
set(CMAKE_CXX_STANDARD 11)

aux_source_directory(. SRCS_DIR)

include_directories(${PROJECT_SOURCE_DIR}/ffmpeg_build/include)
link_directories(${PROJECT_SOURCE_DIR}/ffmpeg_build/lib)


add_executable(${PROJECT_NAME} ${SRCS_DIR})
target_link_libraries(${PROJECT_NAME} avformat)

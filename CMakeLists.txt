cmake_minimum_required (VERSION 3.16)
project(ffmpeg_demo)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")   
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

# C++11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")  
set(CMAKE_CXX_STANDARD 11)

aux_source_directory(. SRCS_DIR)

# 增加链接库的目录，否则下面就不能直接使用avformat(要加入对应的路径)
# 以后使用这种方式来链接库，先引入库的目录，后面可以直接target库的名字
link_directories("ffmpeg_build/lib")

add_subdirectory(demo_decode)

add_executable(${PROJECT_NAME} ${SRCS_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/ffmpeg_build/include)
target_link_libraries(${PROJECT_NAME} avformat)

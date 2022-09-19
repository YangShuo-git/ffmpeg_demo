cmake_minimum_required (VERSION 2.8)
project(FFPLAYER)

# set(CMAKE_BUILD_TYPE "Debug")
# set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")   
# set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
# C++11
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")  
# set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_CXX_FLAGS "-static ${CMAKE_CXX_FLAGS}")

aux_source_directory(. DIR_SRCS)

include_directories(${PROJECT_SOURCE_DIR}/3rdparty/ffmpeg-4.2.1/)
include_directories(${PROJECT_SOURCE_DIR}/ffmpeg_build/include/SDL2/)
link_directories(${PROJECT_SOURCE_DIR}/ffmpeg_build/lib/)

link_directories("/usr/local/lib")
link_directories("/usr/lib")
link_directories("/usr/lib/x86_64-linux-gnu/")

# 指定生成目标 
add_executable(${PROJECT_NAME}  ${DIR_SRCS})
# 添加链接库
target_link_libraries(${PROJECT_NAME} avformat avcodec avfilter avutil postproc swresample swscale SDL2)

# #libavformat
# target_link_libraries(${PROJECT_NAME}  "z" "m" )
# #ibavfilter
# # target_link_libraries(${PROJECT_NAME}  "pthread" "m" "va" "freetype" "z" "png12" "z" "m" )
# #libswresample
# target_link_libraries(${PROJECT_NAME}  "m")
# #libavutil
target_link_libraries(${PROJECT_NAME}  pthread m va-drm va va-x11 va vdpau X11 m va Xv X11 Xext)
# #libswscale
# target_link_libraries(${PROJECT_NAME}  "m" )
# #libx264
# #target_link_libraries(${PROJECT_NAME}  pthread "m" dl)
# #libx265
#target_link_libraries(${PROJECT_NAME}  "stdc++" "m" "rt" "dl" "numa" )

target_link_libraries(${PROJECT_NAME} /usr/lib/x86_64-linux-gnu/libdl.so)

# target_link_libraries(${PROJECT_NAME} /usr/lib/gcc/x86_64-linux-gnu/5/libstdc++.so m rt dl /usr/lib/x86_64-linux-gnu/libnuma.so)



target_link_libraries(${PROJECT_NAME} vorbisenc vorbis  pthread m z stdc++ rt va)





# target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/libavformat.a)
# target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/libavutil.a)
# target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/libavcodec.a)
# target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/libswresample.a)
# target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/libpostproc.a)
# # target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/libavformat.a)
# # target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/libavformat.a)
# target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/libSDL2.a)
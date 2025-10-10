set(PROJECT_NAME "demo_muxer")
project(${PROJECT_NAME})


aux_source_directory(. SRCS_FILE)
add_library(${PROJECT_NAME} ${SRCS_FILE})

target_include_directories(${PROJECT_NAME} PUBLIC ../3rdparty/ffmpeg_build/include)
target_include_directories(${PROJECT_NAME} PUBLIC ../3rdparty/libyuv/include)
target_include_directories(${PROJECT_NAME} PUBLIC ../3rdparty/libx264/include)
target_include_directories(${PROJECT_NAME} PUBLIC ../3rdparty/libfaac/include)
target_include_directories(${PROJECT_NAME} PUBLIC ../3rdparty/libfdk-aac/include)


target_link_libraries(${PROJECT_NAME} avformat 
                                      avcodec 
                                      avutil 
                                      swresample
                                      x264 
                                      fdk-aac 
                                      faac 
                                      libyuv)
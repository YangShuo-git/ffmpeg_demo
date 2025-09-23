set(PROJECT_NAME "demo_muxer")
project(${PROJECT_NAME})


aux_source_directory(. SRCS_FILE)
add_library(${PROJECT_NAME} ${SRCS_FILE})

target_include_directories(${PROJECT_NAME} PUBLIC ../ffmpeg_build/include)
target_link_libraries(${PROJECT_NAME} avformat avcodec avutil swresample)
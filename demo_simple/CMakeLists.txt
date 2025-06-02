project(demo_simple)

aux_source_directory(. SRCS_FILE)
add_executable(${PROJECT_NAME} ${SRCS_FILE})

# target到ffmpeg的头文件与库，二者缺一不可
target_include_directories(${PROJECT_NAME} PUBLIC ../ffmpeg_build/include)
target_link_libraries(${PROJECT_NAME} avformat avcodec avutil)




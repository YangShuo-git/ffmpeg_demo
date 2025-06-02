
add_subdirectory(cppcommon)

# aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/cppcommon COMMON_SRC)  这种方式不可以

aux_source_directory(. SRCS_FILE)
add_executable(demo_decode ${SRCS_FILE})

# target到ffmpeg的头文件与库，二者缺一不可
target_include_directories(demo_decode PUBLIC ../ffmpeg_build/include)
target_link_libraries(demo_decode avformat avcodec avutil)

# 增加依赖库：common(另外一个文件夹也想使用common库，应该怎么办呢？)
# 为什么上面的那种方式不行呢？
target_link_libraries(demo_decode common)




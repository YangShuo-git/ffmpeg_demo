
add_subdirectory(cppcommon)

# aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/cppcommon COMMON_SRC)  这种方式不可以

aux_source_directory(. SRCS_FILE)
add_executable(demo_decode ${SRCS_FILE})

# target到ffmpeg的头文件与库，二者缺一不可
target_include_directories(demo_decode PUBLIC ../ffmpeg_build/include)
target_link_libraries(demo_decode avformat avcodec avutil)

# cppcommon文件夹中的cpp文件，要以库的形式来链接，以项目的方式是失败的（为什么？）
# 因为上面增加了cppcommon的目录，所以不必再target头文件（存疑？）
target_link_libraries(demo_decode common)





aux_source_directory(. SRCS_FILE)

# 在最里层的目录，和target_include_directories没有区别
include_directories(../../ffmpeg_build/include)
add_library(common STATIC ${SRCS_FILE})








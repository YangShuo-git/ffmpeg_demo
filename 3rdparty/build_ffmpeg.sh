cd ffmpeg-4.2.1

./configure  --prefix="$HOME/ffmpeg_demo/ffmpeg_build"   \
--extra-cflags="-I$HOME/ffmpeg_demo/ffmpeg_build/include" \
--extra-ldflags="-L$HOME/ffmpeg_demo/ffmpeg_build/lib" \
--extra-libs="-lpthread -lm" \
--bindir="$HOME/ffmpeg_demo/ffmpeg_build/bin" \
--enable-shared \
--enable-gpl \
--enable-pic \
--enable-nonfree \
--enable-libfreetype \
--enable-libx264 \
--enable-libx265 \
--enable-libfdk-aac \
--enable-libmp3lame \
--enable-libopus \
--enable-libvpx  \
--enable-debug=3 \
--disable-optimizations \
--disable-asm  \
--disable-stripping \
--disable-ffplay  \
--disable-ffprobe \
--disable-doc

make 
make install

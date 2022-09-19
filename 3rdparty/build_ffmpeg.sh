cd ffmpeg-4.2.1
./configure  --prefix="$HOME/ffmpeg_demo/ffmpeg_build"   \
--extra-cflags="-I$HOME/ffmpeg_demo/ffmpeg_build/include" \
--extra-ldflags="-L$HOME/ffmpeg_demo/ffmpeg_build/lib" \
--extra-libs="-lpthread -lm" \
--bindir="$HOME/ffmpeg/bin" \
--enable-gpl \
--enable-nonfree \
--enable-libfdk-aac \
--enable-libx264 \
--enable-libx265 \
--enable-libmp3lame \
--enable-libopus \
--enable-libvpx \
--enable-debug=3 \
--disable-optimizations \
--disable-asm  \
--disable-stripping

make 
make install

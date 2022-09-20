cd ffmpeg-4.2.1
PKG_CONFIG_PATH="$HOME/ffmpeg_demo/ffmpeg_build/lib/pkgconfig" \
./configure  --prefix="$HOME/ffmpeg_demo/ffmpeg_build"   \
--pkg-config-flags="--static" \
--extra-cflags="-I$HOME/ffmpeg_demo/ffmpeg_build/include" \
--extra-ldflags="-L$HOME/ffmpeg_demo/ffmpeg_build/lib" \
--extra-libs="-lpthread -lm" \
--bindir="$HOME/ffmpeg_demo/ffmpeg_build/bin" \
--enable-gpl \
--enable-nonfree \
--enable-libx264 \
--enable-debug=3 \
--disable-optimizations \
--disable-asm  \
--disable-stripping

make 
make install

#ifndef _DEMO_DECODE_H_
#define _DEMO_DECODE_H_

#include <stdio.h>
#include <string>
#include "cppcommon/CLog.h"
 
#ifdef __cplusplus
extern "C" {
#endif

#include "libavformat/avformat.h"
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>

#ifdef __cplusplus
}
#endif


class SoftDecoder
{
public:
    SoftDecoder();
    ~SoftDecoder();

    // 解码本地MP4等封装后的格式文件
    void decodeFormat(std::string srcPath);
    // 解码aac/h264流的文件
    void decodeStream(std::string srcPath);
    // 编码yuv为h264
    void encodeAsStream(std::string srcPath,std::string dstPath);
};






#endif // _DEMO_DECODE_H_
#include <string>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "libavformat/avformat.h"

#ifdef __cplusplus
}
#endif


const std::string mediaFile = "./assets/2_audio.mp4";

int main(int argc, char *argv[]) {
	
	AVFormatContext* pFormatCtx = nullptr;
    
	//创建解封装上下文 pFormatCtx
	if (avformat_open_input(&pFormatCtx, mediaFile.c_str(), nullptr, nullptr) < 0) {
		printf("avformat_open_input failed\n");
		return -1;
	}

	//读取流信息，该函数会填充pFormatCtx->streams
    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0) {
        printf("avformat_find_stream_info failed\n");
        return -1;
    }

    // dump文件信息
    av_dump_format(pFormatCtx, 0, mediaFile.c_str(), 0);


    // 获取指定索引的视频流、音频流
    int video_stream_idx = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    int audio_stream_idx = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);

    // 二级指针就是存放一级指针的数组(获取到流，就获取到需要的所有信息了)
    AVStream* video_stream = pFormatCtx->streams[video_stream_idx];
    AVStream* audio_stream = pFormatCtx->streams[audio_stream_idx];

    printf("\n");
    printf("video codec_id= %d\n", video_stream->codec->codec_id);
    printf("video codec_name= %s\n", avcodec_get_name(video_stream->codec->codec_id));
    printf("video pix_fmt= %d\n", video_stream->codec->pix_fmt);
    printf("video nb_frames= %lld\n", video_stream->nb_frames);
    printf("video avg_frame_rate= %d\n", video_stream->avg_frame_rate.num/video_stream->avg_frame_rate.den);





    return 0;
}
#include <string>
#include <iostream>

using namespace std;

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
// #include <libswscale/swscale.h>
#include "libavutil/imgutils.h"
}

const string mediaFile = "./assets/2_audio.mp4";

int main(int argc, char *argv[]) {
	
	AVFormatContext* pFormatCtx = nullptr;
	//读取avformat
	if (avformat_open_input(&pFormatCtx, mediaFile.c_str(), nullptr, nullptr) < 0) {
		cout << "avformat_open_input failed" << endl;
		return -1;
	}

	//读取流信息，该函数会填充pFormatCtx->streams
    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0) {
        cout << "avformat_find_stream_info failed" << endl;
        return -1;
    }
    //dump格式信息
    av_dump_format(pFormatCtx, 0, mediaFile.c_str(), 0);

    return 0;
}
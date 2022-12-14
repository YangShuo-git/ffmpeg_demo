#include "demo_decode.h"

SoftDecoder::SoftDecoder()
{
    
}

SoftDecoder::~SoftDecoder()
{
    
}

// 为了防止多次调用造成的crash，这里用指针的指针作为参数
static void releaseSources(AVFormatContext **fmtCtx,AVCodecContext **codecCtx1,AVCodecContext **codecCtx2)
{
    if (fmtCtx && *fmtCtx != NULL) {
        avformat_close_input(fmtCtx);
    }
    if (codecCtx1 && *codecCtx1 != NULL) {
        avcodec_free_context(codecCtx1);
    }
    if (codecCtx1 && *codecCtx1 != NULL) {
        avcodec_free_context(codecCtx2);
    }
}

static void decode(AVCodecContext *codecCtx, AVPacket *packet, AVFrame *frame, std::string str)
{
    if (codecCtx == NULL) return;

    int ret = 0;

    // 由于解码器内部会维护一个缓冲区，所以送入解码器的packet并不是立马就能获取到解码数据，所以这里采取如下机制
    avcodec_send_packet(codecCtx,packet);
    while (true) {
        ret = avcodec_receive_frame(codecCtx, frame);
        // 解码器上下文会有一个解码缓冲区，送入的packet并不是立马能够解码的，
        // 如果返回EAGAIN则代表正在解码中，需要继续送入packet即可 （为什么？）
        // 如果返回AVERROR_EOF则表明已到达文件尾部
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) 
        {
            LOGD("%s avcodec_receive_frame:EAGAIN %d",str.c_str(),ret);
            return;
        } 
        else if (ret == AVERROR_EOF)
        {
            LOGD("%s avcodec_receive_frame:AVERROR_EOF %d",str.c_str(),ret);
            return;
        }
        else if (ret < 0) 
        {
            LOGD("%s decodec %d fail",str.c_str(),packet->stream_index,ret);
            return;
        }
        
        // 解码成功
        static int sum=0;
        sum++;
        LOGD("%s decode sucess sum %d",str.c_str(),sum);

        //TODO 生成YUV、PCM
    }
}

void SoftDecoder::decodeFormat(std::string srcPath)
{
    // 创建解封装上下文
    AVFormatContext *in_fmtCtx = NULL;
    AVCodecContext *a_decoderCtx = NULL;
    AVCodecContext *v_decoderCtx = NULL;
    int a_stream_index = -1;
    int v_stream_index = -1;
    int ret = 0;

    // 打开文件或者网络流  重要的操作是把流存解入封装上下文中
    if ((ret = avformat_open_input(&in_fmtCtx, srcPath.c_str(), NULL, NULL)) < 0) {
        LOGD("avformat_open_input fail %d", ret);
        return;
    }
    
    // 查找流信息，并初始化解封装上下文的相关参数(期间会进行解码尝试) 
    if ((ret = avformat_find_stream_info(in_fmtCtx, NULL)) < 0) {
        LOGD("avformat_find_stream_info fail");
        return;
    }
    
    // 打开流的解码器，如何判断是哪一路流，根据stream->codecpar->codec_type
    // 问题，如果是三路流，则只有i=0和i=2，如何解决？（目前的代码是只取出音视频各自的第一路流）
    for (int i = 0; i<in_fmtCtx->nb_streams; i++) 
    {
        AVStream *stream = in_fmtCtx->streams[i];
        enum AVCodecID codeId = stream->codecpar->codec_id;

        if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && a_stream_index == -1) 
        {
            a_stream_index = i;
            
            // 通过该流的的codeId找到解码器，并创建解码器上下文
            AVCodec *codec = avcodec_find_decoder(codeId);
            a_decoderCtx = avcodec_alloc_context3(codec);
            if (a_decoderCtx == NULL) {
                LOGD("avcodec_alloc_contex3 audio fail");
                releaseSources(&in_fmtCtx,NULL,NULL);
                return;
            }
            // 配置解码器上下文，否则avcodec_open2会失败，来源是流的解码器参数（stream->codecpar）
            // 重要的参数是extradata
            avcodec_parameters_to_context(a_decoderCtx, stream->codecpar);

            // 打开解码器，并关联解码器上下文
            if ((ret = avcodec_open2(a_decoderCtx, codec, NULL)) < 0) {
                LOGD("audio avcodec_open2() fail %d",ret);
                releaseSources(&in_fmtCtx, NULL, NULL);
                return;
            }
            LOGD("a_stream_index index: %d", i);
        }
        
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && v_stream_index == -1) 
        {
            v_stream_index = i;
            
            AVCodec *codec = avcodec_find_decoder(codeId);
            v_decoderCtx = avcodec_alloc_context3(codec);
            if (v_decoderCtx == NULL) {
                LOGD("avcodec_alloc_context3 video fail");
                releaseSources(&in_fmtCtx, &a_decoderCtx, NULL);
                return;
            }
            avcodec_parameters_to_context(v_decoderCtx, stream->codecpar);

            if ((ret = avcodec_open2(v_decoderCtx, codec, NULL)) < 0) 
            {
                LOGD("video avcodec_open2() fail %d",ret);
                releaseSources(&in_fmtCtx,&a_decoderCtx,NULL);
                return;
            }
            LOGD("v_decoderCtx index: %d", i);
        }
    }
    
    LOGD("av_dump_format: 打印文件所有流的信息");
    av_dump_format(in_fmtCtx, 0, srcPath.c_str(), 0);
    
    /////////////////////////////////////////////////////////////////////////////////////////
    //解码流程
    AVFrame  *aframe = av_frame_alloc();
    AVFrame  *vframe = av_frame_alloc();
    AVPacket *packet = av_packet_alloc();
    struct timeval btime;
    struct timeval etime;
    gettimeofday(&btime,  NULL);

    // av_read_frame()函数每次调用时内部都会为AVPacket分配内存用于存储未压缩音视频数据，
    // 所以AVPacket用完后要释放掉相应内存
    while (av_read_frame(in_fmtCtx, packet) >= 0) {
        // 根据AVPacket中的stream_index区分对应的是音频数据还是视频数据;
        // 然后将AVPacket送入对应的解码器进行解码
        AVCodecContext *codecCtx = NULL;
        AVFrame        *frame    = NULL;

        if (packet->stream_index == a_stream_index) {
            codecCtx = a_decoderCtx;
            frame = aframe;
        } else if (packet->stream_index == v_stream_index) {
            codecCtx = v_decoderCtx;
            frame = vframe;
        }
        
        if (codecCtx != NULL) {
            decode(codecCtx, packet, frame, packet->stream_index == a_stream_index ? "audio":"video");
        }
        
        // 释放内存
        av_packet_unref(packet);
    }
    
    // 刷新缓冲区 
    decode(a_decoderCtx, NULL, aframe, "audio");
    decode(v_decoderCtx, NULL, vframe, "video");
    gettimeofday(&etime, NULL);
    LOGD("解码耗时 %.2f s",(etime.tv_sec - btime.tv_sec)+(etime.tv_usec - btime.tv_usec)/1000000.0f);

    releaseSources(&in_fmtCtx, &a_decoderCtx, &v_decoderCtx);
}

// 为了防止多次调用造成的crash，这里用指针的指针作为参数  （为什么？）
static void releaseSources2(AVFormatContext **fmtCtx,AVCodecContext **codecCtx,AVCodecParserContext **parserCtx)
{
    if (fmtCtx && *fmtCtx != NULL) {
        avformat_close_input(fmtCtx);
    }
    if (codecCtx && *codecCtx != NULL) {
        avcodec_free_context(codecCtx);
    }
    if (parserCtx && *parserCtx != NULL) {
        av_parser_close(*parserCtx);
    }
}

/** 熟悉av_parser_parser2()函数的用法
 *  1、同一个AVCodecParserContext只能解析一路流
 */
#define Parser_Buffer_Size 1024*100

void SoftDecoder::decodeStream(std::string srcPath)
{
    // av_parser_parser2()只适合解析裸流：aac流,h264流(这种流一般用于网络播放时，比如基于RTSP，RTMP协议的)，
    // 并不适合本地MP4直接读取后的解析(本地文件还得用AVFormatContext解封装)
    FILE *in_file = NULL;
    uint8_t in_buff[Parser_Buffer_Size + AV_INPUT_BUFFER_PADDING_SIZE];
    uint8_t *data = NULL;
    int data_size;
    data = in_buff;
    data_size = Parser_Buffer_Size;
    bool isAudio = true;
    
    if ((in_file = fopen(srcPath.c_str(), "rb")) == NULL){
        LOGD("fopen fail");
        return;
    }
    
    AVFormatContext *fmt_Ctx = NULL;
    AVCodecContext *decoderCtx = NULL;
    AVCodecParserContext *parserCtx = NULL;
    AVCodec *codec = NULL;

    // 进一步理解解封装上下文是用来存放流的
    if (avformat_open_input(&fmt_Ctx, srcPath.c_str(), NULL, NULL) < 0) {
        LOGD("avformat_open_input fail");
        return;
    }
    enum AVCodecID codeId = fmt_Ctx->streams[0]->codecpar->codec_id;
    isAudio = fmt_Ctx->streams[0]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO;
    codec = avcodec_find_decoder(codeId);
    decoderCtx = avcodec_alloc_context3(codec);
    if (decoderCtx == NULL) {
        LOGD("avcodec_alloc_context3 fail");
        releaseSources2(&fmt_Ctx, NULL, NULL);
        return;
    }
    if (avcodec_open2(decoderCtx, codec, NULL) < 0) {
        LOGD("avcodec_open2 fail");
        releaseSources2(&fmt_Ctx,NULL, NULL);
        return;
    }

    //创建解析器  用于解码裸流
    parserCtx = av_parser_init(codeId);
    if (parserCtx == NULL) {
        LOGD("av_parser_init fail");
        releaseSources2(&fmt_Ctx, &decoderCtx, NULL);
        return;
    }
    
    AVPacket *packet = av_packet_alloc();
    AVFrame  *frame = av_frame_alloc();
    size_t ret = 0;
    while ((ret = fread(data, 1, data_size, in_file)) > 0) {
        
        if (ret < data_size) {
            data_size = (int)ret;
        }
        
        while (data_size > 0) {
            // 解析裸流，将data分割成独立的packet
            int len = av_parser_parse2(parserCtx, decoderCtx, &packet->data, &packet->size, data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
            if (len < 0) {
                LOGD("av_parser_parser2() fail %d",len);
                break;
            }
            data += len;
            data_size -= len;
            
            if (packet->size > 0) {
                decode(decoderCtx, packet, frame, isAudio?"audio":"video");
                av_packet_unref(packet);
            }
        }
        
        //位置复位 重新读取
        data = in_buff;
        data_size = Parser_Buffer_Size;
    }
    
    decode(decoderCtx, NULL, frame, isAudio?"audio":"video");
    
    releaseSources2(&fmt_Ctx, &decoderCtx, &parserCtx);
}

static void encode_frame(AVCodecContext *ctx,AVFrame *frame,FILE *ouFile)
{
    int ret = 0;
    avcodec_send_frame(ctx, frame);
    AVPacket *pkt = av_packet_alloc();
    while (true) {
        ret = avcodec_receive_packet(ctx, pkt);
        if (ret < 0) break;
        static int sum = 0;
        // tips：对于不同的平台，x264编码后每个packet的size大小不一定一样，但总数是一样的
        sum++;
        LOGD("pkt size %d sum %d",pkt->size,sum);
        fwrite(pkt->data, 1, pkt->size, ouFile);
        av_packet_unref(pkt);
    }
}

void SoftDecoder::encodeAsStream(std::string srcPath,std::string dstPath)
{
    FILE *inFile = NULL,*ouFile = NULL;
    int width = 640,height = 360,fps = 50;
    
    AVCodecContext *enc_ctx = NULL;
    AVCodec *codec = NULL;
    int ret = 0;
    codec = avcodec_find_encoder_by_name("libx264");
    if (codec == NULL) {
        LOGD("avcodec_find_encoder_by_name fail");
        return;
    }
    enc_ctx = avcodec_alloc_context3(codec);
    if (enc_ctx == NULL) {
        LOGD("avcodec_alloc_context3 fail");
        return;
    }
    
    // 设置编码参数
    // 原始视频宽度
    enc_ctx->width = width;
    // 原始视频高度
    enc_ctx->height = height;
    // 编码后视频的时间基
    enc_ctx->time_base = (AVRational){1,fps};
    // 视频帧率
    enc_ctx->framerate = (AVRational){fps,1};
    enc_ctx->sample_aspect_ratio = (AVRational){1, 1};
    // 视频码率；单位bit/s H264各个分辨率推荐的码率表:http://www.lighterra.com/papers/videoencodingh264/
    enc_ctx->bit_rate = 0.96*1000000;
    // 视频像素格式
    enc_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    // 视频I帧间隔
    enc_ctx->gop_size = 10;
    if (codec->capabilities & AV_CODEC_FLAG_LOW_DELAY) {
        LOGD("AV_CODEC_FLAG_LOW_DELAY");
    }

    // x264编码特有的参数
    if (enc_ctx->codec_id == AV_CODEC_ID_H264) {
        av_opt_set(enc_ctx->priv_data,"reset","slow",0);
        enc_ctx->flags |= AV_CODEC_FLAG2_LOCAL_HEADER;
    }
    
    // 初始化编码器上下文
    /** 遇到问题：执行此函数时，崩溃(报错stack_not_16_byte_aligned_error;
     *  分析原因：mac osX 10.15.4 (19E266)和Version 11.4 (11E146)生成的库在调用libx264编码的avcodec_open2()函数有问题。
     *  解决方案：添加编译参数--disable-optimizations解决问题(fix：2020.5.2)
     *  备注：用ffplay播放生成的.h264文件时提示"56109 segmentation fault"，也是同样的问题，重新编译ffmpeg生成ffplay即可
     */
    if ((ret = avcodec_open2(enc_ctx,codec,NULL)) < 0) {
        LOGD("avcodec_open2 fail");
        avcodec_free_context(&enc_ctx);
        return;
    }
    
    inFile = fopen(srcPath.c_str(), "rb");
    ouFile = fopen(dstPath.c_str(), "wb+");
    if (inFile == NULL) {
        LOGD("open src file fail");
        return;
    }
    if (ouFile == NULL) {
        LOGD("open dst file fail");
        return;
    }
    
    AVFrame  *frame = av_frame_alloc();
    frame->width = width;
    frame->height = height;
    frame->format = enc_ctx->pix_fmt;
    /** 遇到问题：编码后h264文件播放视频画面有绿条
     *  分析原因：源yuv为480x640的yuv420p方式存储时，由于源yuv文件yuv数据存储时不是按字节对齐方式存储的，而这里创建的AVFrame又是按照
     *  字节对齐的方式分配内存的即linesize的大小和480不对应，导致数据错乱。
     *  解决方案：当从源yuv文件读取数据到AVFrame时linesize要和yuv的宽对应，所以这里要将av_frame_get_buffer()的第二个参数设置为1
     */
    av_frame_get_buffer(frame, 1);
    av_frame_make_writable(frame);
    int frame_size = width * height;
    int frame_count = 0;
    while (1) {
        if (enc_ctx->pix_fmt == AV_PIX_FMT_YUV420P) {
            // 读取数据之前先清掉之前数据
            memset(frame->data[0], 0, frame_size);
            memset(frame->data[1], 0, frame_size/4);
            memset(frame->data[2], 0, frame_size/4);
            // 读取Y数据
            if(fread(frame->data[0], 1, frame_size, inFile) <= 0) break;
            // 读取U数据
            if(fread(frame->data[1], 1, frame_size/4, inFile) <= 0) break;
            // 读取V数据
            if(fread(frame->data[2], 1, frame_size/4, inFile) <= 0) break;
            
        } else if (enc_ctx->pix_fmt == AV_PIX_FMT_NV12 || enc_ctx->pix_fmt == AV_PIX_FMT_NV21) {
            // 读取数据之前先清掉之前数据
            memset(frame->data[0], 0, frame_size);
            memset(frame->data[1], 0, frame_size/2);
            // 读取Y数据
            if(fread(frame->data[0], 1, frame_size, inFile) <= 0) break;
            // 读取UV数据
            if(fread(frame->data[1], 1, frame_size/2, inFile) <= 0) break;
        }
        frame->pts = frame_count;
        frame_count++;
        encode_frame(enc_ctx, frame,ouFile);
    }
    
    // 刷新编码缓冲区中的数据
    encode_frame(enc_ctx,NULL,ouFile);
    
    // 释放资源
    avcodec_free_context(&enc_ctx);
    av_frame_unref(frame);
    fclose(inFile);
    fclose(ouFile);
}

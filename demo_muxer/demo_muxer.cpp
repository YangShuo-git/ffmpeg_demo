#include "demo_muxer.h"
#include "libyuv.h"
#include <sys/time.h>

Muxer*  Muxer::m_pInstance = NULL;
pthread_mutex_t Muxer::m_InstanceMutex;
Muxer::Garbage  Muxer::m_garbage;

Muxer* Muxer::getInstance()
{
    if(m_pInstance == nullptr){
        pthread_mutex_init(&m_InstanceMutex, 0);

        pthread_mutex_lock(&m_InstanceMutex);
        m_pInstance = new Muxer();
        pthread_mutex_unlock(&m_InstanceMutex);
    }

    return m_pInstance;
}

Muxer::Muxer()
{
    av_register_all();
    avcodec_register_all();

    pthread_mutex_init(&m_videoWriteMutex, 0);
    pthread_mutex_init(&m_aacWriterMutex, 0);

    m_startTimeStamp = 0;
}


bool Muxer::startRecord(const char* file)
{
    if(file == NULL){
        return false;
    }

    m_pFormatCtx     = NULL;
    m_pAudioCodecCtx = NULL;
    m_pVideoCodecCtx = NULL;
    m_pYUVFrame      = NULL;

    m_filePath = file;

    //1.创建output Format
    avformat_alloc_output_context2(&m_pFormatCtx, NULL, NULL, file);
    if(m_pFormatCtx == NULL){
        cout<<"avformat_alloc_output_context2 failed!"<<endl;
        return false;
    }

    //2.添加Video、audio码流
    addVideoStream();
    addAudioStream();

    //3.写文件头
    if(!writeFileHeader()){
        writeFileTail();

        releaseRecordResources();
        cout<<"write video header failed!"<<endl;
        return false;
    }

    m_bRecording = true;
    m_startTimeStamp = getTickCount();

    return true;
}

void Muxer::stopRecord()
{
    m_bRecording = false;

    writeFileTail();
    releaseRecordResources();
}

void Muxer::releaseRecordResources()
{
    if(m_pAudioCodecCtx != NULL){
        avcodec_free_context(&m_pAudioCodecCtx);
    }
    if(m_pVideoCodecCtx != NULL){
        avcodec_free_context(&m_pVideoCodecCtx);
    }
    if(m_pFormatCtx != NULL){
        avformat_free_context(m_pFormatCtx);
    }
    if(m_pYUVFrame != NULL){
        av_frame_free(&m_pYUVFrame);
    }
}

bool Muxer::writeFileHeader()
{
    if(!m_pFormatCtx){
        return false;
    }

    // IO文件句柄
    int ret = avio_open(&m_pFormatCtx->pb, m_filePath.c_str(), AVIO_FLAG_WRITE);
    if(ret != 0){
        cout << "avio_open failed!"<<endl;
        return false;
    }
    ret = avformat_write_header(m_pFormatCtx, NULL);
    if(ret != 0){
        cout << "avformat_write_header failed!"<<endl;
        return false;
    }

    cout <<"Write file: "<<m_filePath<<" header success!"<<endl;
    return true;
}

bool Muxer::writeFileTail()
{
    if(!m_pFormatCtx){
        return false;
    }

    if(av_write_trailer(m_pFormatCtx) != 0){
        cout <<"av_write_trailer failed!"<<endl;
        return false;
    }

    if(avio_closep(&m_pFormatCtx->pb) != 0){
         cout <<"avio_closep failed!"<<endl;
        return false;
    }

    cout <<"Write file tail success !"<<endl;
    return true;
}

bool Muxer::addVideoStream()
{
    if(m_pFormatCtx == NULL){
        return false;
    }

    AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if(!codec){
        cout <<"avcodec_find_encoder AV_CODEC_ID_H264 failed!"<<endl;
        return false;
    }

    m_pVideoCodecCtx = avcodec_alloc_context3(codec);
    if(!m_pVideoCodecCtx){
        cout <<"video avcodec_alloc_context3 failed!"<<endl;
        return false;
    }

    m_pVideoCodecCtx->width    = m_videoOutWidth;
    m_pVideoCodecCtx->height   = m_videoOutHeight;
    m_pVideoCodecCtx->pix_fmt  = AV_PIX_FMT_YUV420P;
    m_pVideoCodecCtx->codec_id = AV_CODEC_ID_H264;

    m_pVideoCodecCtx->time_base = (AVRational){1,30}; //codec的时间基和帧率基本一致 
    m_pVideoCodecCtx->gop_size  = 20; //20帧一个关键帧
    m_pVideoCodecCtx->max_b_frames = 0; //b帧为0
    m_pVideoCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER; //全局头，不是每帧都有头

    //设置编码相关的参数,需要avdevice.h
    av_opt_set(m_pVideoCodecCtx->priv_data, "preset", "superfast", 0); 
    

    int ret = avcodec_open2(m_pVideoCodecCtx, codec, NULL);
    if(ret != 0){
        avcodec_free_context(&m_pVideoCodecCtx);
        cout <<"video avcodec_open2 failed!"<<endl;
        return false;
    }

    //最终目标是new stream 并填充该流的codecpar
    m_pVideoStream = avformat_new_stream(m_pFormatCtx, NULL);
    if(!m_pVideoStream){
        cout <<"avformat_new_stream failed!"<<endl;
        return false;
    }
    m_pVideoStream->codecpar->codec_tag = 0;//默认值为0，直接由CodecId决定
    avcodec_parameters_from_context(m_pVideoStream->codecpar, m_pVideoCodecCtx);

    //编码需要用到的yuv frame
    if(m_pYUVFrame == NULL){
        m_pYUVFrame = av_frame_alloc();

        m_pYUVFrame->pts = 0;
        m_pYUVFrame->width  = m_videoOutWidth;
        m_pYUVFrame->height = m_videoOutHeight;
        m_pYUVFrame->format = AV_PIX_FMT_YUV420P;

        //32字节对齐充分利用现代CPU的AVX/AVX2指令集，兼容所有主流SIMD指令
        //获得最佳的视频处理性能，符合FFmpeg的最佳实践
        int ret = av_frame_get_buffer(m_pYUVFrame, 32);
        if(ret != 0){
            cout <<"av_frame_get_buffer failed!"<<endl;
            return false;
        }
    }

    return true;
}

bool Muxer::addAudioStream()
{
    if(m_pFormatCtx == NULL){
        return false;
    }

    AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if(!codec){
        cout <<"avcodec_find_encoder AV_CODEC_ID_AAC failed!"<<endl;
        return false;
    }

    m_pAudioCodecCtx = avcodec_alloc_context3(codec);
    if(!m_pAudioCodecCtx){
        cout <<"audio avcodec_alloc_context3 failed!"<<endl;
        return false;
    }

    m_pAudioCodecCtx->channels    = m_audioOutChannels;
    m_pAudioCodecCtx->sample_rate = m_audioOutSamplerate;
    m_pAudioCodecCtx->sample_fmt  = AV_SAMPLE_FMT_FLTP;
    m_pAudioCodecCtx->channel_layout = av_get_default_channel_layout(m_audioOutChannels);
    m_pAudioCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    int ret = avcodec_open2(m_pAudioCodecCtx, codec, NULL);
    if(ret != 0){
        avcodec_free_context(&m_pAudioCodecCtx);
        cout <<"audio avcodec_open2 failed!"<<endl;
        return false;
    }

    m_pAudioStream = avformat_new_stream(m_pFormatCtx, NULL);
    if(!m_pAudioStream){
        cout<<"audio avformat_new_stream failed"<<endl;
        return false;
    }
    m_pAudioStream->codecpar->codec_tag =0;
    avcodec_parameters_from_context(m_pAudioStream->codecpar, m_pAudioCodecCtx);
    
    return true;
}

bool Muxer::writeFrame(AVPacket* pkt)
{
    if(!m_bRecording){
        return false;
    }
    if(!m_pFormatCtx || !pkt || pkt->size <= 0){
        return false;
    }
    if(pkt->data == NULL){
        return false;
    }

    pthread_mutex_lock(&m_videoWriteMutex);
    int retValue= av_interleaved_write_frame(m_pFormatCtx, pkt);
    pthread_mutex_unlock(&m_videoWriteMutex);
    if(retValue != 0){
        cout << "av_interleaved_write_frame failed!"<<endl;
        return false;
    }

    return true;
}

bool Muxer::writeVideoFrameWithRgbData(unsigned char* rgb)
{
    if(!m_bRecording){
        return false;
    }
    if(!m_pFormatCtx || !m_pYUVFrame || !rgb){
        return false;
    }

    int t_width  = m_videoInWidth;
    int t_height = m_videoInHeight;

    int rgbStride = m_videoInWidth * 4;
    int whSize    = t_width * t_height;
    int uv_stride = t_width / 2;
    int uv_length = uv_stride * (t_height/2);

    uint8_t* yuvBuffer = (uint8_t*)malloc(whSize * 3 / 2);

    uint8_t* Y_data_Ptr = yuvBuffer;
    uint8_t* U_data_ptr = yuvBuffer + whSize;
    uint8_t* V_data_ptr = yuvBuffer + whSize + uv_length;

    libyuv::BGRAToI420((uint8_t*)rgb, rgbStride, 
                                Y_data_Ptr, t_width, 
                                U_data_ptr, uv_stride, 
                                V_data_ptr, uv_stride, 
                                t_width, t_height);

    m_pYUVFrame->data[0] = Y_data_Ptr;
    m_pYUVFrame->data[1] = U_data_ptr;
    m_pYUVFrame->data[2] = V_data_ptr;

    m_pYUVFrame->linesize[0] = t_width;
    m_pYUVFrame->linesize[1] = uv_stride;
    m_pYUVFrame->linesize[2] = uv_stride;

    //1、设置frame的pts
    unsigned long currentPts = getTickCount() - m_startTimeStamp;
    if(currentPts <=0){
        currentPts +=1;
    }
    m_pYUVFrame->pts = currentPts;

    //2、编码
    int ret = avcodec_send_frame(m_pVideoCodecCtx, m_pYUVFrame);
    if(ret != 0){
        if(yuvBuffer != NULL){
            free(yuvBuffer);
            yuvBuffer = NULL;
        }
        return false;
    }

    //处理退出的情况
    if(!m_bRecording || !m_pFormatCtx || !m_pYUVFrame){
        return false;
    }

    AVPacket packet;
    av_init_packet(&packet);
    ret = avcodec_receive_packet(m_pVideoCodecCtx, &packet);
    if(ret !=0 || packet.size <= 0){
       if(yuvBuffer != NULL){
            free(yuvBuffer);
            yuvBuffer = NULL;
        }
        return false;     
    }

    if(yuvBuffer != NULL){
        free(yuvBuffer);
        yuvBuffer = NULL;
    }

    //3、转换时间基：frame timebase --> stream timebase
    av_packet_rescale_ts(&packet, m_pVideoCodecCtx->time_base, m_pVideoStream->time_base);
    //4、设置流索引
    packet.stream_index = m_pVideoStream->index;

    //5、写入mp4
    writeFrame(&packet);

    return true;
}


////////////////////////////////////////////////
unsigned long Muxer::getTickCount()
{
    struct timeval tv;
    if(gettimeofday(&tv,NULL ) != 0){
        return 0;
    }
    //返回ms
    return (tv.tv_sec*1000 + tv.tv_usec/1000);
}
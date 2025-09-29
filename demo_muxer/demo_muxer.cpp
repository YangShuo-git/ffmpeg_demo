#include "demo_muxer.h"
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


bool Muxer::startRecordWithFilePath(const char* file)
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

    //2.写Video、audio码流
    //TODO

    //3.写文件头
    if(!writeVideoHeader()){
        writeMp4FileTail();

        releaseAllRecordResources();
        cout<<"write video header failed!"<<endl;
        return false;
    }

    m_bRecording = true;
    m_startTimeStamp = getTickCount();

    return true;
}

void Muxer::stopRecordReleaseAllResources()
{
    m_bRecording = false;

    writeMp4FileTail();
    releaseAllRecordResources();
}

void Muxer::releaseAllRecordResources()
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

bool Muxer::writeVideoHeader()
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

bool Muxer::writeMp4FileTail()
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
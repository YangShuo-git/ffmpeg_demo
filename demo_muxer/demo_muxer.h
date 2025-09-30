#ifndef _DEMO_MUXER_H_
#define _DEMO_MUXER_H_

extern "C"
{
#include <libavformat/avformat.h>
// #include <libavdevice/avdevice.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
#include <libavcodec/avcodec.h>
}

#include <pthread.h>
#include <iostream>
// #include "faac.h"  //ffmpeg3.0以后可能没有这个

using namespace std;

typedef struct AAC_CONFIGURE
{
    // faacEncHandle hEncoder;            //音频文件描述符

    unsigned int nSampleRate;          //音频采样数
    unsigned int nChannels;  	       //音频声道数
    unsigned int nPCMBitSize;          //音频采样精度

    unsigned int nInputSamples;        //每次调用编码时所应接收的原始数据长度
    unsigned int nMaxOutputBytes;      //每次调用编码时生成的AAC数据的最大长度
    unsigned char* pcmBuffer;          //pcm数据
    unsigned char* aacBuffer;          //aac数据
}AACEncodeConfig;

class Muxer
{
public:
    static Muxer* getInstance();

    bool startRecord(const char* file);
    void stopRecord();

    void setupInputResolution(int w, int h)   { m_videoInWidth = w; m_videoInHeight = h;     }
    void setupOutputResolution(int w, int h)  { m_videoOutWidth = w; m_videoOutHeight = h;   }
    void setupVideoOutParms(int fps, int bit) { m_videoOutFps = fps; m_videoOutBitrate = bit; }

    bool writeVideoFrameWithRgbData(unsigned char* rgb);
    bool writeAudioFrameWithPcmData(unsigned char* data, int size);

private:
    int m_videoInWidth  = 640;
    int m_videoInHeight = 360;

    int m_videoOutWidth   = 640;
    int m_videoOutHeight  = 360;
    int m_videoOutBitrate = 512000;
    int m_videoOutFps     = 15;

    int m_audioOutBitrate    = 640000;
    int m_audioOutSamplerate = 44100;
    int m_audioOutChannels   = 2;

    int m_samples = 960; //输入输出的每帧数据每通道的样本数

    int m_audioFramePts = 0;
    int m_videoFramePts = 0;

    string m_filePath   = "";
    bool   m_bRecording = false;

    pthread_mutex_t         m_videoWriteMutex;
    pthread_mutex_t         m_aacWriterMutex;

    unsigned long           m_lastAudioTimeStamp;
    unsigned long           m_currentAudioTimeStamp;

    int                     m_pcmBufferSize=0;
    int                     m_pcmBufferRemainSize=0;
    int                     m_pcmWriteRemainSize=0;

    AACEncodeConfig*        m_aacEncodeConfig;


    unsigned long           m_startTimeStamp;

    AVFormatContext*        m_pFormatCtx     = nullptr;

    AVCodecContext*         m_pVideoCodecCtx = nullptr;
    AVCodecContext*         m_pAudioCodecCtx = nullptr;

    AVStream*               m_pVideoStream   = nullptr;
    AVStream*               m_pAudioStream   = nullptr;

    AVFrame*                m_pYUVFrame      = nullptr;

private:
    bool    addVideoStream();
    bool    addAudioStream();

    bool    writeFrame(AVPacket* pkt);

    bool    writeFileHeader();
    bool    writeFileTail();

    void    releaseResources();

    AACEncodeConfig* initAudioEncodeConfiguration();
    void releaseAccConfiguration();
    int linearPCM2AAC(unsigned char * pData,int captureSize);

    //获取不断增长的时间ms，用于设置PTS
    unsigned long getTickCount();

private:
    Muxer();

    static Muxer* m_pInstance;
    static pthread_mutex_t m_InstanceMutex;

    //回收单例资源
    class Garbage{
        public:
            ~Garbage(){
                if(Muxer::m_pInstance != nullptr){
                    delete Muxer::m_pInstance;
                    Muxer::m_pInstance= NULL;
                }
            }
    };

    static Garbage m_garbage;
};

#endif // _DEMO_MUXER_H_
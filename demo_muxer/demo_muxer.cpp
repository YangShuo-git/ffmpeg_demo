#include "demo_muxer.h"
#include <sys/time.h>

Muxer*  Muxer::m_pInstance = NULL;
pthread_mutex_t Muxer::m_mutex;
Muxer::Garbage  Muxer::m_garbage;

Muxer* Muxer::GetInstance()
{
    if(m_pInstance == nullptr){
        pthread_mutex_init(&m_mutex, 0);

        pthread_mutex_lock(&m_mutex);
        m_pInstance = new Muxer();
        pthread_mutex_unlock(&m_mutex);
    }

    return m_pInstance;
}
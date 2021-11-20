
#ifndef _AUDIO_WEBRTC_BUFFER_H
#define _AUDIO_WEBRTC_BUFFER_H

#if defined(__cplusplus)
extern "C"
{
#endif

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct AudioWebrtcBuffer AudioWebrtcBuffer;
typedef struct AudioWebrtcBufferParam AudioWebrtcBufferParam;

int Audio_Webrtc_Buffer_Create(AudioWebrtcBuffer **handle,
                               AudioWebrtcBufferParam *parameter);
int Audio_Webrtc_Buffer_Destroy(AudioWebrtcBuffer *handle);
int Audio_Webrtc_Buffer_Size(AudioWebrtcBuffer *handle,
                             size_t *length);
int Audio_Webrtc_Buffer_PopBack(AudioWebrtcBuffer *handle,
                                int16_t *buffer, size_t length); // ReadFromEnd
int Audio_Webrtc_Buffer_PopFront(AudioWebrtcBuffer *handle,
                                 int16_t *buffer, size_t length); // Read(FromBegin)
int Audio_Webrtc_Buffer_PushBack(AudioWebrtcBuffer *handle,
                                 const int16_t *buffer, size_t length); // Write(ToEnd)
int Audio_Webrtc_Buffer_PushFront(AudioWebrtcBuffer *handle,
                                  const int16_t *buffer, size_t length); // WriteToBegin

struct AudioWebrtcBufferParam
{
    int num_channels_;
};

#if defined(__cplusplus)
};
#endif

#endif /* _AUDIO_WEBRTC_BUFFER_H */

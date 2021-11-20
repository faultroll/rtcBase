
#include "audio_webrtc_buffer.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "modules/audio_coding/neteq/audio_multi_vector.h"

// TODO |sync_buffer_|
/* // |Buffer_Size| is used as parameter in |buffer_level_filter_|->Update
// |Buffer_ReadFromEnd| is used to borrow samples to acquire |required_samples_|
struct AudioWebrtcBuffer
{
    void *buffer_handle_;
    // callbacks
    size_t (*Buffer_Size)(void *); // used size, Size()
    void (*Buffer_ReadFromEnd)(void *, void *, size_t); // borrow, PopBack()
    void (*Buffer_ReadFromBegin)(void *, void *, size_t); // read, PopFront()
    void (*Buffer_WriteToEnd)(void *, size_t, size_t); // write, PushBack()
    void (*Buffer_WriteToBegin)(void *, size_t, size_t); // insert, PushFront()
}; */

struct AudioWebrtcBuffer
{
    webrtc::AudioMultiVector *buffer_;
};

int Audio_Webrtc_Buffer_Create(AudioWebrtcBuffer **handle,
                               AudioWebrtcBufferParam *parameter)
{
    if (NULL == handle
        || NULL == parameter)
    {
        return -1;
    }

    int return_code;
    AudioWebrtcBuffer *handle_tmp = NULL;
    do
    {
        handle_tmp =
            (AudioWebrtcBuffer *)malloc(sizeof(AudioWebrtcBuffer));
        if (NULL == handle_tmp)
        {
            return_code = -1;
            break;
        }

        handle_tmp->buffer_ =
            new webrtc::AudioMultiVector(parameter->num_channels_);
        if (NULL == handle_tmp->buffer_)
        {
            free(handle_tmp);
            handle_tmp = NULL;
            return_code = -1;
            break;
        }

        return_code = 0;
    }
    while (0);

    *handle = handle_tmp;

    return return_code;
}

int Audio_Webrtc_Buffer_Destroy(AudioWebrtcBuffer *handle)
{
    if (NULL == handle)
    {
        return -1;
    }

    // TODO check |buffer_|

    int return_code;
    do
    {
        delete handle->buffer_;
        free(handle);

        return_code = 0;
    }
    while (0);

    return return_code;
}

int Audio_Webrtc_Buffer_Size(AudioWebrtcBuffer *handle,
                             size_t *length)
{
    if (NULL == handle
        || NULL == length)
    {
        return -1;
    }

    // TODO check |buffer_|

    int return_code;
    do
    {
        *length = handle->buffer_->Size();

        return_code = 0;
    }
    while (0);

    return return_code;
}

int Audio_Webrtc_Buffer_PopBack(AudioWebrtcBuffer *handle,
                                int16_t *buffer, size_t length)
{
    if (NULL == handle
        || NULL == buffer || 0 == length)
    {
        return -1;
    }

    // TODO check |buffer_|

    int return_code;
    do
    {
        handle->buffer_->ReadInterleavedFromEnd(length, buffer);
        handle->buffer_->PopBack(length);

        return_code = 0;
    }
    while (0);

    return return_code;
}

int Audio_Webrtc_Buffer_PopFront(AudioWebrtcBuffer *handle,
                                 int16_t *buffer, size_t length)
{
    if (NULL == handle
        || NULL == buffer || 0 == length)
    {
        return -1;
    }

    // TODO check |buffer_|

    int return_code;
    do
    {
        handle->buffer_->ReadInterleaved(length, buffer);
        handle->buffer_->PopFront(length);

        return_code = 0;
    }
    while (0);

    return return_code;
}

int Audio_Webrtc_Buffer_PushBack(AudioWebrtcBuffer *handle,
                                 const int16_t *buffer, size_t length)
{
    if (NULL == handle
        || NULL == buffer || 0 == length)
    {
        return -1;
    }

    // TODO check |buffer_|

    int return_code;
    do
    {
        handle->buffer_->PushBackInterleaved(buffer, length);

        return_code = 0;
    }
    while (0);

    return return_code;
}

int Audio_Webrtc_Buffer_PushFront(AudioWebrtcBuffer *handle,
                                  const int16_t *buffer, size_t length)
{
    if (NULL == handle
        || NULL == buffer || 0 == length)
    {
        return -1;
    }

    // TODO check |buffer_|

    int return_code;
    do
    {
        // TODO no func
        // handle->buffer_->PushFront(buffer, length);

        return_code = 0;
    }
    while (0);

    return return_code;
}

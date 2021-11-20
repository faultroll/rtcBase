#include<stdio.h>
#include"webrtc/modules/audio_conference_mixer/include/audio_conference_mixer.h"
// #include"webrtc/modules/audio_conference_mixer/include/audio_conference_mixer_defines.h"

static const int iSampleRate = 48000;
static const int iChannels = 2;
static const int iBit = 16;

class AudioCallback : public webrtc::AudioMixerOutputReceiver
{
public:
    virtual void NewMixedAudio(const int32_t id,
                               const webrtc::AudioFrame &generalAudioFrame,
                               const webrtc::AudioFrame **uniqueAudioFrames,
                               const uint32_t size)
    {
        // printf("callback Len=%d\n", size);

        m_audioFrame.CopyFrom(generalAudioFrame);
    }
public:
    webrtc::AudioFrame m_audioFrame;
};

class Participant : public webrtc::MixerParticipant
{
public:
    Participant(int i) : m_id(i)
    {

    }

    virtual int32_t GetAudioFrame(int32_t id, webrtc::AudioFrame *audioFrame)
    {
        // printf("GetAudioFrame id=%d\n", id);

        audioFrame->CopyFrom(m_audioFrame);

        return 0;
    }

    virtual int32_t NeededFrequency(int32_t id) const
    {
        return iSampleRate;
    }

    void filldata(short *pdata, int iSample)
    {
        m_audioFrame.id_ = m_id;
        m_audioFrame.num_channels_ = iChannels;
        m_audioFrame.samples_per_channel_ = iSampleRate / (1000 / 10);
        m_audioFrame.sample_rate_hz_ = iSampleRate;
        m_audioFrame.speech_type_ = webrtc::AudioFrame::kNormalSpeech;
        m_audioFrame.vad_activity_ = webrtc::AudioFrame::kVadActive;

        memcpy(m_audioFrame.data_, pdata, iSample * sizeof(short));
    }

public:
    webrtc::AudioFrame m_audioFrame;
    int m_id;
};

int main(int argc, char *argv[])
{
    const int i10msSz = iSampleRate * iChannels * (iBit / 8)
                        / (1000 / 10) / sizeof(short); // 10ms Sample, short

    FILE *pSrc1 = nullptr, *pSrc2 = nullptr, *pSrc3 = nullptr,
          *pSrc4 = nullptr, *pSrc5 = nullptr, *pSrc6 = nullptr;
    FILE *pOut = nullptr;

    pSrc1 = fopen("1_4800_16_2.pcm", "rb+");
    pSrc2 = fopen("2_4800_16_2.pcm", "rb+");
    pSrc3 = fopen("3_4800_16_2.pcm", "rb+");
    pSrc4 = fopen("4_4800_16_2.pcm", "rb+");
    pSrc5 = fopen("5_4800_16_2.pcm", "rb+");
    pSrc6 = fopen("6_4800_16_2.pcm", "rb+");
    pOut = fopen("conferencemix.pcm", "wb+");

    int iId = 0;
    short *pBuf1 = new short[i10msSz];
    short *pBuf2 = new short[i10msSz];
    short *pBuf3 = new short[i10msSz];
    short *pBuf4 = new short[i10msSz];
    short *pBuf5 = new short[i10msSz];
    short *pBuf6 = new short[i10msSz];
    webrtc::AudioConferenceMixer *pMixer = webrtc::AudioConferenceMixer::Create(iId);

    Participant *pPar1 = new Participant(1);
    Participant *pPar2 = new Participant(2);
    Participant *pPar3 = new Participant(3);
    Participant *pPar4 = new Participant(4);
    Participant *pPar5 = new Participant(5);
    Participant *pPar6 = new Participant(6);

    pMixer->SetMixabilityStatus(pPar1, false);
    pMixer->SetMixabilityStatus(pPar2, true);
    pMixer->SetMixabilityStatus(pPar3, true);
    pMixer->SetMixabilityStatus(pPar4, false);
    pMixer->SetMixabilityStatus(pPar5, false);
    pMixer->SetMixabilityStatus(pPar6, false);

    AudioCallback *pCb = new AudioCallback;

    pMixer->RegisterMixedStreamCallback(pCb);

    int iR1 = 0, iR2 = 0, iR3 = 0, iR4 = 0, iR5 = 0, iR6 = 0;
    int64_t timeLeft = 0, mixCount = 0;

    do
    {
        if ((timeLeft = pMixer->TimeUntilNextProcess()) != 0)
        {
            // printf("left (%lld) ms\n", timeLeft);

            continue;
        }
        else
        {
            // printf("mix (%lld) times\n", ++mixCount); // mixCount * i10msSz * sizeof(short) / 1024 == filesize of pOut

            iR1 = fread(pBuf1, sizeof(short), i10msSz, pSrc1);
            iR2 = fread(pBuf2, sizeof(short), i10msSz, pSrc2);
            iR3 = fread(pBuf3, sizeof(short), i10msSz, pSrc3);
            iR4 = fread(pBuf4, sizeof(short), i10msSz, pSrc4);
            iR5 = fread(pBuf5, sizeof(short), i10msSz, pSrc5);
            iR6 = fread(pBuf6, sizeof(short), i10msSz, pSrc6);

            pPar1->filldata(pBuf1, iR1);
            pPar2->filldata(pBuf2, iR2);
            pPar3->filldata(pBuf3, iR3);
            pPar4->filldata(pBuf4, iR4);
            pPar5->filldata(pBuf5, iR5);
            pPar6->filldata(pBuf6, iR6);

            pMixer->Process();

            fwrite(pCb->m_audioFrame.data_, sizeof(short), iR1, pOut);
        }
    }
    while (iR1 && iR2 && iR3 && iR4 && iR5 && iR6);

    pMixer->UnRegisterMixedStreamCallback();
    delete pPar1;
    delete pPar2;
    delete pPar3;
    delete pPar4;
    delete pPar5;
    delete pPar6;
    delete pMixer;

    fclose(pSrc1);
    fclose(pSrc2);
    fclose(pSrc3);
    fclose(pSrc4);
    fclose(pSrc5);
    fclose(pSrc6);
    fclose(pOut);

    return 0;
}
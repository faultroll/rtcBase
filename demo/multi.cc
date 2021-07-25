
// From https://blog.csdn.net/kenny_zh/article/details/38592775

#include <string>
#include <iostream>
#include "rtc_base/thread.h"

class HelpData : public rtc::MessageData
{
public:
    std::string info_;
};

class Police : public rtc::MessageHandler
{
public:
    enum {
        MSG_HELP,
    };

    void Help(rtc::Thread &targetThread, const std::string &info)
    {
        HelpData *data = new HelpData;
        data->info_ = info;
        targetThread.Post(RTC_FROM_HERE, this, MSG_HELP, data);
    }

    virtual void OnMessage(rtc::Message *msg)
    {
        switch (msg->message_id) {
            case MSG_HELP:
                HelpData *data = (HelpData *)msg->pdata;
                std::cout << "MSG_HELP :" << data->info_ << std::endl;
                break;
        }
    }
};

int main(void)
{
    std::cout << "Test Multi-thread is started" << std::endl;
    Police p;
    rtc::Thread thread;
    thread.Start();

    p.Help(thread, "Please help me!");
    rtc::Thread::Current()->SleepMs(100);
    std::cout << "Test Multi-thread is completed" << std::endl;

    return 0;
}
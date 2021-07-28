
// From https://blog.csdn.net/kenny_zh/article/details/38592735

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

    void Help(const std::string info)
    {
        HelpData *data = new HelpData;
        data->info_ = info;
        rtc::Thread::Current()->Post(RTC_FROM_HERE, this, MSG_HELP, data);
    }

    virtual void OnMessage(rtc::Message *msg)
    {
        switch (msg->message_id) {
            case MSG_HELP:
                HelpData *data = (HelpData *)msg->pdata;
                std::cout << "MSG_HELP : " << data->info_ << std::endl;
                break;
        }

        delete msg->pdata;
    }
};

int main(void)
{
    std::cout << "Test Single-thread is started" << std::endl;
    Police p;
    p.Help("Please help me!");
    rtc::Thread::Current()->Run();
    std::cout << "Test Thread is completed" << std::endl;

    // won't be reached
    // rtc::ThreadManager::Instance()->UnwrapCurrentThread();

    return 0;
}

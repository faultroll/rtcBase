
#include <iostream>
#include "rtc_base/thread.h"

// From rtc_base/signalthread.h, use |thread| instead of |platform_thread|

class Worker : public rtc::Thread
{
public:
    explicit Worker()
        : Thread(rtc::SocketServer::CreateDefault()),
          count_(0) {}
    ~Worker() override {};
    void Run() override
    {
        while (!IsQuitting()) {
            count_++;
            std::cout << "zZz..zZz.." << count_ << "...zZz..zZz" << std::endl;
            SleepMs(200);
        }
    }
    bool IsProcessingMessages() override
    {
        return false;
    }

private:
    size_t count_;

    RTC_DISALLOW_COPY_AND_ASSIGN(Worker);
};


int main(void)
{
    Worker worker_;
    worker_.SetName("Override");
    worker_.Start();
    rtc::Thread::Current()->SleepMs(1010); // |count_| should be 1000/200+1=6, first one at 0ms
    worker_.Stop();
    rtc::ThreadManager::Instance()->UnwrapCurrentThread();

    return 0;
}

#include <iostream>
#include "rtc_base/thread.h"

// TODO(s)
// 1. service Post to adapter, adapter use callback to tell main
// 2. different service same adapter, relation between adapter and service

class Adapter : public rtc::MessageHandler
{
public:
    Adapter()
    {
        thread_ = rtc::Thread::Create();
        thread_->Start();
    }
    ~Adapter()
    {
        thread_->Stop();
    }

    enum operation {
        START,
        STOP,
    };

    class StartData : public rtc::MessageData
    {
    public:
        size_t which_;
        std::string info_;
    };

    class StopData : public rtc::MessageData
    {
    public:
        size_t which_;
    };

    void AsyncMsg(enum operation oper, void *data)
    {
        thread_->Post(RTC_FROM_HERE, this,
                      oper, static_cast<rtc::MessageData *>(data));
    }

    virtual int SyncMsg(enum operation oper, void *data)
    {
        int result = 0;
        switch (oper) {
            case START: {
                rtc::Thread::Current()->SleepMs(1000); // block caller for 1 sec
                StartData *data_tmp = (StartData *)data;
                std::cout << "START: " << data_tmp->which_ <<
                          ": " << data_tmp->info_ << std::endl;
                break;
            }
            case STOP: {
                StopData *data_tmp = (StopData *)data;
                std::cout << "STOP: " << data_tmp->which_ << std::endl;
                break;
            }
            default : {
                std::cout << "WHAT IS THIS" << std::endl;
                result = -1;
                break;
            }
        }
        delete static_cast<rtc::MessageData *>(data); // should be deleted here

        return result;
    }

    virtual void OnMessage(rtc::Message *msg)
    {
        SyncMsg(static_cast<enum operation>(msg->message_id), msg->pdata);
    }
private:
    std::unique_ptr<rtc::Thread> thread_;
};

class Service : public rtc::MessageHandler
{
public:
    Service()
        : working_(false),
          count_(0)
    {
        thread_ = rtc::Thread::Create();
        thread_->Start();
    }
    ~Service()
    {
        thread_->Stop();
    }

    enum operation {
        INIT,
        WORK,
        REWORK,
        FINISH,
    };

    class InitData : public rtc::MessageData
    {
    public:
        size_t which_;
    };

    class FinishData : public rtc::MessageData
    {
    public:
        size_t which_;
    };

    void SendMsg(enum operation oper, void *data)
    {
        thread_->Post(RTC_FROM_HERE, this,
                      oper, static_cast<rtc::MessageData *>(data));
    }

    virtual void OnMessage(rtc::Message *msg)
    {
        switch (msg->message_id) {
            case INIT: {
                InitData *data = (InitData *)msg->pdata;
                std::cout << "INIT: " << data->which_ << std::endl;
                // First blood for worker
                working_ = true;
                thread_->PostDelayed(RTC_FROM_HERE, 200, this, REWORK, nullptr); // Post also works
                break;
            }
            case WORK: {
                if (working_) {
                    std::cout << "WORK" << std::endl;
                }
                break;
            }
            case REWORK: {
                if (working_) {
                    count_++;
                    std::cout << "WORK-WORK: " << count_ << std::endl;
                    thread_->PostDelayed(RTC_FROM_HERE, 200, this, REWORK, nullptr);
                }
                break;
            }
            case FINISH: {
                FinishData *data = (FinishData *)msg->pdata;
                if (thread_->IsCurrent()) {
                    // No race
                    working_ = false;
                    count_ = 0;
                    std::cout << "FINISH: " << data->which_ << std::endl;
                } else {
                    // cannot happen
                    std::cout << "FINISH: something wrong" << std::endl;
                }
                break;
            }
        }

        delete msg->pdata;
    }
private:
    std::unique_ptr<rtc::Thread> thread_;
    bool working_;
    size_t count_;
};

int main(void)
{
    Adapter *adapter = new Adapter;

    Adapter::StartData *start = new Adapter::StartData;
    start->which_ = 123;
    start->info_ = "balabala";
    adapter->AsyncMsg(Adapter::START, start);
    std::cout << "async will not block this" << std::endl;

    Adapter::StopData *stop = new Adapter::StopData;
    stop->which_ = start->which_;
    int result = adapter->SyncMsg(Adapter::STOP, stop);
    std::cout << "sync will block this, result: " << result << std::endl;

    Service *service = new Service;
    Service::InitData *init = new Service::InitData;
    init->which_ = 233;
    service->SendMsg(Service::INIT, init);
    rtc::Thread::Current()->SleepMs(1000); // |count_| should be 1000/200=5, first one at 200ms
    service->SendMsg(Service::WORK, nullptr);
    rtc::Thread::Current()->SleepMs(200); // |REWORK| will not block |WORK|
    service->SendMsg(Service::WORK, nullptr);
    Service::FinishData *finish = new Service::FinishData;
    finish->which_ = 233;
    service->SendMsg(Service::FINISH, finish);
    service->SendMsg(Service::WORK, nullptr);
    service->SendMsg(Service::REWORK, nullptr);

    delete service;
    delete adapter;
    rtc::ThreadManager::Instance()->UnwrapCurrentThread();

    return 0;
}
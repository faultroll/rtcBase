
#include <iostream>
#include "rtc_base/thread.h"

// TODO(s)
// - service Post to adapter, adapter use callback to tell main
// - different service same adapter, relation between adapter and service
// - service&adapter一个线程两个handle，adapter直接send给service（同一线程会直接调用），成员thread *current_，自发生成job，成员inchn同样位置存储job handle控制开关d
// - 三种方式增加：单次（阻塞/非阻塞）/多次（定时，起停）
// - 没有adapter线程，拆分命令直接onmessage内调，不分成两个线程；service纯虚类，提供thread_以及asyncmsg&synccall两个纯虚方法，不同service直接继承并提供enum/data和handler_，同时子service为单例
// - 不同service互不影响（encoder/merger)
// - 同一service顺序响应，sync不能直接调（除非支持多线程）要用send；自发命令替代status；直接调回调代替report
// - 能力集按子系统（service）报则post否则直接调回调
// - service小修仅不继承thread，在onmessage中直接调用proc
// - 按proj分达到定制，还是不同service分，不同proj仅对参数定制：不同对外接口subproc，单独一个类（全集，枚举集，调service），然后不同proj根据需要调用subproc（同一类对外接口其实一个就够？）；问题如何达到现在这种只编译需要的？只能通过判断enum来控制，但还是编译进去了



/* class Adapter : public rtc::MessageHandler
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
}; */

class Service // can cascade
{
public:
    Service(rtc::Thread *thread)
        : thread_(thread),
          handler_(this) {}
    virtual ~Service() {}

    void AsyncMsg(int oper, void *data)
    {
        ServiceData *data_tmp = new ServiceData;
        data_tmp->oper_ = oper;
        data_tmp->data_ = data; // dup data
        data_tmp->result_ = nullptr;
        thread_->Post(RTC_FROM_HERE, &handler_,
                      ServiceHandler::kProcess, data_tmp);
    }

    int SyncCall(int oper, void *data)
    {
        int result;

        ServiceData *data_tmp = new ServiceData;
        data_tmp->oper_ = oper;
        data_tmp->data_ = data;
        data_tmp->result_ = &result;
        thread_->Send(RTC_FROM_HERE, &handler_,
                      ServiceHandler::kProcess, data_tmp);

        return result;
    }

    virtual int Process(int oper, void *data) = 0;

private:
    class WorkHandler : public rtc::MessageData
    {
    public:
        int oper_;
        void *data_;
        bool working_;
    };

    // should be public
    WorkHandler *BeginWork(int oper, void *data)
    {
        WorkHandler *handler = new WorkHandler;
        handler->oper_ = oper;
        handler->data_ = data; // dup data
        handler->working_ = true;
        thread_->Post(RTC_FROM_HERE, &handler_,
                      ServiceHandler::kWorkWork, handler);
        return handler;
    }

    void WorkWork(WorkHandler *handler)
    {
        Process(handler->oper_, handler->data_);
        thread_->PostDelayed(RTC_FROM_HERE, 200, &handler_,
                             ServiceHandler::kWorkWork, handler);
    }

    // should be public
    void EndWork(WorkHandler *handler)
    {
        handler->working_ = false;
        // handler deleted in OnMessage
    }

    class ServiceData : public rtc::MessageData
    {
    public:
        int oper_;
        void *data_;
        int *result_;
    };

    class ServiceHandler : public rtc::MessageHandler
    {
    public:
        enum Operations {
            kProcess,
            kWorkWork,
        };
        ServiceHandler(Service *parent)
            : parent_(parent) {}
        /* virtual */ ~ServiceHandler() {}

        /* virtual */ void OnMessage(rtc::Message *msg)
        {
            switch (msg->message_id) {
                case kWorkWork: {
                    WorkHandler *handler = (WorkHandler *)msg->pdata;
                    if (handler->working_) {
                        std::cout << "WORK~WORK~" << std::endl;
                        parent_->WorkWork(handler);
                    } else {
                        delete handler;
                    }
                    break;
                }
                default: {
                    ServiceData *data = (ServiceData *)msg->pdata;
                    int result = parent_->Process(msg->message_id, msg->pdata);
                    if (nullptr == data->result_) {
                        // sync
                        *data->result_ = result;
                    } else {
                        // async
                        // free data
                    }
                    delete msg->pdata;
                    break;
                }
            }
        }

        Service *parent_;
    };

    rtc::Thread *thread_;
    ServiceHandler handler_;
};

int main(void)
{
    /* Adapter *adapter = new Adapter;

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
    rtc::Thread::Current()->SleepMs(
        1000); // |count_| should be 1000/200=5, first one at 200ms
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
    rtc::ThreadManager::Instance()->UnwrapCurrentThread(); */

    return 0;
}
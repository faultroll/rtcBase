
#include <iostream>
#include "rtc_base/thread.h"

// TODO(s)
// - service Post to adapter, adapter use callback to tell main
// - different service same adapter, relation between adapter and service
// - service&adapter一个线程两个handle，adapter直接send给service（同一线程会直接调用），成员thread *current_，自发生成job，成员inchn同样位置存储job handle控制开关d
// - 三种方式增加：单次（阻塞/非阻塞）/多次（定时，起停）
// - 没有adapter线程，拆分命令直接onmessage内调，不分成两个线程；子service为单例
// - 不同service互不影响（encoder/merger)
// - 同一service顺序响应，sync不能直接调（除非支持多线程）要用send；自发命令替代status；直接调回调代替report
// - 能力集按子系统（service）报则post否则直接调回调
// - service小修仅不继承thread，在onmessage中直接调用proc
// - 定制: 不同proj仅对参数定制，不同对外接口（如zk1, zk2）subproc，单独一个类（全集，枚举集，调service），然后不同proj根据需要调用subproc；如何达到现在这种只编译需要的？


class Peon // can cascade
{
public:
    Peon(rtc::Thread *thread)
        : thread_(thread),
          handler_(this) {}
    virtual ~Peon() {}

    /* virtual */ void AsyncMsg(int oper, void *data)
    {
        WorkData *handler = new WorkData;
        handler->oper_ = oper;
        handler->data_ = DataDup(oper, data);
        handler->result_ = nullptr;
        thread_->Post(RTC_FROM_HERE, &handler_,
                      PeonHandler::kWork, handler);
    }

    /* virtual */ int SyncCall(int oper, void *data)
    {
        int result;

        WorkData *handler = new WorkData;
        handler->oper_ = oper;
        handler->data_ = data;
        handler->result_ = &result;
        thread_->Send(RTC_FROM_HERE, &handler_,
                      PeonHandler::kWork, handler);

        return result;
    }

    virtual void *DataDup(int oper, void *data) = 0;
    virtual void DataFree(int oper, void *data) = 0;
    virtual int Process(int oper, void *data) = 0;
    virtual void Report(int oper, void *data, int result) = 0;

private:
    class WorkData : public rtc::MessageData
    {
    public:
        int oper_;
        void *data_;
        int *result_;
    };

    class WorkWorkData : public rtc::MessageData
    {
    public:
        int oper_;
        void *data_;
        bool working_;
        int rest_;
    };

    // should be public
    WorkWorkData *ReadyToWork(int oper, void *data, int rest)
    {
        WorkWorkData *handler = new WorkWorkData;
        handler->oper_ = oper;
        handler->data_ = DataDup(oper, data);
        handler->working_ = true;
        handler->rest_ = rest;
        thread_->Post(RTC_FROM_HERE, &handler_,
                      PeonHandler::kWorkWork, handler);
        return handler;
    }

    void WorkWork(WorkWorkData *handler)
    {
        thread_->PostDelayed(RTC_FROM_HERE, handler->rest_, &handler_,
                             PeonHandler::kWorkWork, handler);
    }

    // should be public
    void MoreWork(WorkWorkData *handler)
    {
        handler->working_ = false;
        // |handler| is deleted in OnMessage
    }

    class PeonHandler : public rtc::MessageHandler
    {
    public:
        enum Operations {
            kWork,
            kWorkWork,
        };
        PeonHandler(Peon *parent)
            : parent_(parent) {}
        /* virtual */ ~PeonHandler() {}

        /* virtual */ void OnMessage(rtc::Message *msg)
        {
            switch (msg->message_id) {
                case kWork: {
                    WorkData *handler = (WorkData *)msg->pdata;
                    /* std::cout << "READY-TO-WORK..." << std::endl; */
                    int result = parent_->Process(handler->oper_, handler->data_);
                    if (handler->result_ != nullptr) {
                        // sync
                        *handler->result_ = result;
                    } else {
                        // async
                        parent_->Report(handler->oper_, handler->data_, result);
                        parent_->DataFree(handler->oper_, handler->data_);
                    }
                    delete handler;
                    break;
                }
                case kWorkWork: {
                    WorkWorkData *handler = (WorkWorkData *)msg->pdata;
                    if (handler->working_) {
                        /* std::cout << "WORK~WORK~" << std::endl; */
                        parent_->Process(handler->oper_, handler->data_);
                        parent_->WorkWork(handler);
                    } else {
                        /* std::cout << "MORE-WORK!" << std::endl; */
                        parent_->DataFree(handler->oper_, handler->data_);
                        delete handler;
                    }
                    break;
                }
                default: {
                    break;
                }
            }
        }

        Peon *parent_;
    };

    rtc::Thread *thread_;
    PeonHandler handler_;
};

#include <string.h>

class Service : public Peon
{
public:
    Service(rtc::Thread *thread)
        : Peon(thread) {}
    virtual ~Service() {}

    void *DataDup(int oper, void *data)
    {
        (void)oper;
        size_t size = 1024;
        void *data_tmp = malloc(size);
        memmove(data_tmp, data, size);
        return data_tmp;
    }

    void DataFree(int oper, void *data)
    {
        (void)oper;
        free(data);
    }

    enum Operations {
        kHengHeng,
        kHaHa,
    };

    struct HengHengData {
        int heng_;
    };

    struct HaHaData {
        int ha_;
    };

    int Process(int oper, void *data)
    {
        std::cout << "Process Service: " << oper << std::endl;

        switch (oper) {
            case kHengHeng: {
                HengHengData *data_tmp = (HengHengData *)data;
                std::cout << "Heng " << data_tmp->heng_ << std::endl;
                break;
            }
            case kHaHa: {
                HaHaData *data_tmp = (HaHaData *)data;
                std::cout << "Ha " << data_tmp->ha_ << std::endl;
                break;
            }
            default: {
                break;
            }
        }

        return 0xdeadbeaf;
    }

    void Report(int oper, void *data, int result)
    {
        (void)oper;
        (void)data;
        std::cout << "Report Service: " << std::hex << result << std::endl;
    }
};

class Adapter : public Peon
{
public:
    Adapter(rtc::Thread *thread, Service *service)
        : Peon(thread),
          service_(service) {}
    virtual ~Adapter() {}

    void *DataDup(int oper, void *data)
    {
        (void)oper;
        size_t size = 1024;
        void *data_tmp = malloc(size);
        memmove(data_tmp, data, size);
        return data_tmp;
    }

    void DataFree(int oper, void *data)
    {
        (void)oper;
        free(data);
    }

    int Process(int oper, void *data)
    {
        (void)oper;
        (void)data;
        std::cout << "Process Adapter: " << oper << std::endl;
        service_->SyncCall(oper, data);

        return 0xdeadbeaf;
    }

    void Report(int oper, void *data, int result)
    {
        (void)oper;
        (void)data;
        std::cout << "Report Adapter: " << std::hex << result << std::endl;
    }

    // // error: marked ‘override’, but does not override
    // int SyncCall(int oper, void *data) /* override */
    // {
    //     return Process(oper, data);
    // }

private:
    Service *service_;
};

class Project1
{
public :
    Project1()
    {
        thread_ = rtc::Thread::Create();
        service_ = new Service(thread_.get());
        adapter_ = new Adapter(thread_.get(), service_);
        thread_->Start();
    }
    ~Project1()
    {
        delete adapter_;
        delete service_;
        thread_->Stop();
    }

    Adapter *adapter_;
    Service *service_;

private :
    std::unique_ptr<rtc::Thread> thread_;
};

int main(void)
{
    Project1 project1;
    void *data = malloc(111);
    // project1.adapter_->AsyncMsg(213, data);
    // project1.adapter_->SyncCall(233, data);
    // project1.service_->AsyncMsg(213, data);
    // project1.service_->SyncCall(233, data);
    *(int *)data = 1024;
    project1.adapter_->AsyncMsg(Service::kHengHeng, data);
    project1.adapter_->SyncCall(Service::kHaHa, data); // bug: data is changed, why?
    free(data);

    rtc::ThreadManager::Instance()->UnwrapCurrentThread();

    return 0;
}
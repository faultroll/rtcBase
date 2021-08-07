
#include <iostream>
#include "rtc_base/thread.h"

// - |project|--|adapter|--|service|
//   - 1 |project| 1 |adapter|
//     |project| should be singleton, same |project| means same api
//     |adapter| is needed because of |AsyncMsg| and different api
//     1 |project| may have m |adapter| instance, they(also |service|) should be compiled as needed
//   - 1 |adapter| m |service|
//     1 |service| and 1adapter instance in 1 thread
//     |adapter| should be relative with api(project), while |service| shouldn't
//     |service| should be singleton, different |service| is isolate

class Peon // can cascade
{
public:
    Peon(int type, rtc::Thread *thread)
        : type_(type),
          thread_(thread),
          handler_(this) {}
    virtual ~Peon() {}

    /* virtual */ void AsyncMsg(int oper, void *data)
    {
        OnePushData *handler = new OnePushData(this, oper, data, nullptr);
        thread_->Post(RTC_FROM_HERE, &handler_,
                      PeonHandler::kOnePush, handler);
    }

    /* virtual */ int SyncCall(int oper, void *data)
    {
        int result;

        OnePushData *handler = new OnePushData(this, oper, data, &result);
        thread_->Send(RTC_FROM_HERE, &handler_,
                      PeonHandler::kOnePush, handler);

        return result;
    }

    virtual void *DataDupFunction(int oper, void *data) = 0;
    virtual void DataFreeFunction(int oper, void *data) = 0;
    virtual int ProcessFunction(int oper, void *data) = 0;
    virtual void ReportFunction(int oper, void *data, int result) = 0;

    // should be private
    class AutoCycleData : public rtc::MessageData
    {
    public:
        AutoCycleData(Peon *parent, int oper, void *data, int interval)
            : oper_(oper),
              data_(data),
              cycling_(true),
              interval_(interval),
              //   quit_(false),
              parent_(parent)
        {
            data_ = parent_->DataDupFunction(oper_, data_);
        }
        ~AutoCycleData()
        {
            parent_->DataFreeFunction(oper_, data_);
        }

        int oper_;
        void *data_;
        bool cycling_;
        int interval_;
        // bool quit_;

    private:
        Peon *parent_;
    };

    AutoCycleData *EnterCycle(int oper, void *data, int interval)
    {
        AutoCycleData *handler = new AutoCycleData(this, oper, data, interval);
        thread_->Post(RTC_FROM_HERE, &handler_,
                      PeonHandler::kAutoCycle, handler);
        return handler;
    }

    // should be private
    void ToNextCycle(AutoCycleData *handler)
    {
        thread_->PostDelayed(RTC_FROM_HERE, handler->interval_, &handler_,
                             PeonHandler::kAutoCycle, handler);
    }

    void ExitCycle(AutoCycleData *handler)
    {
        handler->cycling_ = false;
        // |handler| is deleted in |OnMessage|
        /* if (thread_->IsCurrent()) {
            // cannot |Clear| if more than one |AutoCycleData|
            // thread_->Clear(&handler_, PeonHandler::kAutoCycle);
        } else {
            // should wait until |OnMessage| done
            while (!handler->quit_) {
                rtc::Thread::Current()->SleepMs(handler->interval_);
            }
        }
        delete handler; */
    }

    int type_;

private:
    class OnePushData : public rtc::MessageData
    {
    public:
        OnePushData(Peon *parent, int oper, void *data, int *result)
            : oper_(oper),
              data_(data),
              result_(result),
              parent_(parent)
        {
            if (result_ != nullptr) {
                ; // do nothing
            } else {
                data_ = parent_->DataDupFunction(oper_, data_);
            }
        }
        ~OnePushData()
        {
            if (result_ != nullptr) {
                ; // do nothing
            } else {
                // should be here, otherwise memleak will occur when |Stop|
                parent_->DataFreeFunction(oper_, data_);
            }
        }

        int oper_;
        void *data_;
        int *result_;

    private:
        Peon *parent_;
    };

    class PeonHandler : public rtc::MessageHandler
    {
    public:
        enum Operations {
            kOnePush,
            kAutoCycle,
        };
        PeonHandler(Peon *parent)
            : parent_(parent) {}
        /* virtual */ ~PeonHandler() {}

        /* virtual */ void OnMessage(rtc::Message *msg)
        {
            switch (msg->message_id) {
                case kOnePush: {
                    OnePushData *handler = (OnePushData *)msg->pdata;
                    int result = parent_->ProcessFunction(handler->oper_, handler->data_);
                    if (handler->result_ != nullptr) {
                        // sync
                        *handler->result_ = result;
                    } else {
                        // async
                        parent_->ReportFunction(handler->oper_, handler->data_, result);
                    }
                    delete handler;
                    break;
                }
                case kAutoCycle: {
                    AutoCycleData *handler = (AutoCycleData *)msg->pdata;
                    if (handler->cycling_) {
                        parent_->ProcessFunction(handler->oper_, handler->data_);
                        parent_->ToNextCycle(handler);
                    } else {
                        // handler->quit_ = true;
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

//

#include <string.h>

enum Services {
    kAlpha,
    kDelta,
};

class ServiceAlpha : public Peon
{
public:
    // singleton
    ServiceAlpha *Instance(rtc::Thread *thread)
    {
        static ServiceAlpha *const service = new ServiceAlpha(thread);
        return service;
    }

    ServiceAlpha(rtc::Thread *thread)
        : Peon(kAlpha, thread) {}
    virtual ~ServiceAlpha() {}

    enum Operations {
        kHeng,
        kHa,
        kHengHa,
    };

    struct HengData {
        int heng_;
    };

    struct HaData {
        int ha_;
    };

    struct HengHaData {
        int hengha_;
    };

    void *DataDupFunction(int oper, void *data)
    {
        const struct {
            enum Operations oper_;
            size_t size_;
        } tbl_sz[] = {
            {kHeng, sizeof(HengData)},
            {kHa, sizeof(HaData)},
            {kHengHa, sizeof(HengHaData)},
        };
        const int len = (int)(sizeof(tbl_sz) / sizeof(tbl_sz[0]));
        int i;
        size_t size;
        for (i = 0; i < len; i++)
            if (tbl_sz[i].oper_ == oper)
                break;
        if (i < len)
            size = tbl_sz[i].size_;
        else
            size = 1024;
        void *data_tmp = malloc(size);
        memmove(data_tmp, data, size);
        std::cout << "DataDupFunction Service" << type_ << ": "
                  << oper << ", size: " << size << ", data: " << data_tmp << std::endl;
        return data_tmp;
    }

    void DataFreeFunction(int oper, void *data)
    {
        std::cout << "DataFreeFunction Service" << type_ << ": "
                  << oper << ", data: " << data << std::endl;
        free(data);
    }

    int ProcessFunction(int oper, void *data)
    {
        std::cout << "ProcessFunction Service" << type_ << ": "
                  << oper << std::endl;

        switch (oper) {
            case kHeng: {
                HengData *data_tmp = (HengData *)data;
                std::cout << "Heng: " << data_tmp->heng_ << std::endl;
                HengHaData count;
                count.hengha_ = 0;
                handler_ = EnterCycle(kHengHa, &count, 200);
                break;
            }
            case kHa: {
                HaData *data_tmp = (HaData *)data;
                std::cout << "Ha: " << data_tmp->ha_ << std::endl;
                ExitCycle(handler_);
                break;
            }
            case kHengHa: {
                HengHaData *data_tmp = (HengHaData *)data;
                data_tmp->hengha_++;
                std::cout << "HengHa! " << data_tmp->hengha_ << std::endl;
                break;
            }
            default: {
                break;
            }
        }

        return 0xdeadbeaf;
    }

    void ReportFunction(int oper, void *data, int result)
    {
        (void)oper;
        (void)data;
        printf("ReportFunction Service%d: %#x\n", type_, result);
    }

private:
    AutoCycleData *handler_;
};

//

enum Adapters {
    kAAA,
    kBBB,
};

class AdapterAAA : public Peon
{
public:
    AdapterAAA(rtc::Thread *thread, ServiceAlpha *service)
        : Peon(kAAA, thread),
          service_(service) {}
    virtual ~AdapterAAA() {}

    enum Operations {
        kHengHeng,
        kHaHa,
    };

    struct HengHengData {
        int hengheng_;
    };

    struct HaHaData {
        int haha_;
    };

    void *DataDupFunction(int oper, void *data)
    {
        const struct {
            enum Operations oper_;
            size_t size_;
        } tbl_sz[] = {
            {kHengHeng, sizeof(HengHengData)},
            {kHaHa, sizeof(HaHaData)},
        };
        const int len = (int)(sizeof(tbl_sz) / sizeof(tbl_sz[0]));
        int i;
        size_t size;
        for (i = 0; i < len; i++)
            if (tbl_sz[i].oper_ == oper)
                break;
        if (i < len)
            size = tbl_sz[i].size_;
        else
            size = 1024;
        void *data_tmp = malloc(size);
        memmove(data_tmp, data, size);
        std::cout << "DataDupFunction Adapter" << type_ << ": "
                  << oper << ", size: " << size << ", data: " << data_tmp << std::endl;
        return data_tmp;
    }

    void DataFreeFunction(int oper, void *data)
    {
        std::cout << "DataFreeFunction Adapter" << type_ << ": "
                  << oper << ", data: " << data << std::endl;
        free(data);
    }

    int ProcessFunction(int oper, void *data)
    {
        std::cout << "ProcessFunction Adapter" << type_ << ": "
                  << oper << std::endl;

        switch (oper) {
            case kHengHeng: {
                HengHengData *data_src = (HengHengData *)data;
                ServiceAlpha::HengData *data_dst = new ServiceAlpha::HengData;
                data_dst->heng_ = data_src->hengheng_;
                service_->SyncCall(ServiceAlpha::kHeng, data_dst);
                delete data_dst;
                break;
            }
            case kHaHa: {
                HaHaData *data_src = (HaHaData *)data;
                ServiceAlpha::HaData *data_dst = new ServiceAlpha::HaData;
                data_dst->ha_ = data_src->haha_;
                service_->SyncCall(ServiceAlpha::kHa, data);
                delete data_dst;
                break;
            }
            default: {
                break;
            }
        }

        return 0xdeadbeaf;
    }

    void ReportFunction(int oper, void *data, int result)
    {
        (void)oper;
        (void)data;
        // bug: data is changed when using |std::hex|, for it changes all numerics to hex after using it
        // std::cout << "ReportFunction Adapter: " << std::hex << result << std::endl;
        printf("ReportFunction Adapter%d: %#x\n", type_, result);
    }

    // error: marked ‘override’, but does not override
    int SyncCall(int oper, void *data) /* override */
    {
        return ProcessFunction(oper, data);
    }

private:
    ServiceAlpha *service_;
};

class Project
{
public :
    Project()
    {
        // thread_ = rtc::Thread::Create();
        // service_ = new Service(thread_.get());
        // adapter_ = new Adapter(thread_.get(), service_);
        thread_ = new rtc::Thread(rtc::SocketServer::CreateDefault());
        service_ = new ServiceAlpha(thread_);
        adapter_ = new AdapterAAA(thread_, service_);
        thread_->Start();
    }
    ~Project()
    {
        thread_->Stop();
        // will cause segment fault if |thread_| is deleted after |service_|
        // once |service_| is deleted, the |DataFreeFunction| funcs and |parent_| in |Peon| will be *Wild*
        // thus segment fault occurs when |AutoCycleData| dtor
        // thread_ = nullptr; // delete |std::unique_ptr|
        delete thread_;
        delete adapter_;
        delete service_;
    }

    AdapterAAA *adapter_;
    ServiceAlpha *service_;

private :
    // std::unique_ptr<rtc::Thread> thread_;
    rtc::Thread *thread_;
};

int main(void)
{
    Project project;
    void *data = malloc(111);
    *(int *)data = 1024;
    project.adapter_->AsyncMsg(AdapterAAA::kHengHeng, data);
    rtc::Thread::Current()->SleepMs(1000); // |hengha_| should be 1000/200+1=6
    project.adapter_->SyncCall(AdapterAAA::kHaHa, data);
    free(data);

    rtc::ThreadManager::Instance()->UnwrapCurrentThread();

    return 0;
}
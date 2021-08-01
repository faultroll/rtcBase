
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
        WorkData *handler = new WorkData(this, oper, data, nullptr);
        thread_->Post(RTC_FROM_HERE, &handler_,
                      PeonHandler::kWork, handler);
    }

    /* virtual */ int SyncCall(int oper, void *data)
    {
        int result;

        WorkData *handler = new WorkData(this, oper, data, &result);
        thread_->Send(RTC_FROM_HERE, &handler_,
                      PeonHandler::kWork, handler);

        return result;
    }

    virtual void *DataDup(int oper, void *data) = 0;
    virtual void DataFree(int oper, void *data) = 0;
    virtual int Process(int oper, void *data) = 0;
    virtual void Report(int oper, void *data, int result) = 0;

    // should be private
    class WorkWorkData : public rtc::MessageData
    {
    public:
        WorkWorkData(Peon *parent, int oper, void *data, int rest)
            : oper_(oper),
              data_(data),
              working_(true),
              rest_(rest),
              //   quit_(false),
              parent_(parent)
        {
            data_ = parent_->DataDup(oper_, data_);
        }
        ~WorkWorkData()
        {
            parent_->DataFree(oper_, data_);
        }

        int oper_;
        void *data_;
        bool working_;
        int rest_;
        // bool quit_;

    private:
        Peon *parent_;
    };

    WorkWorkData *ReadyToWork(int oper, void *data, int rest)
    {
        WorkWorkData *handler = new WorkWorkData(this, oper, data, rest);
        thread_->Post(RTC_FROM_HERE, &handler_,
                      PeonHandler::kWorkWork, handler);
        return handler;
    }

    // should be private
    void WorkWork(WorkWorkData *handler)
    {
        thread_->PostDelayed(RTC_FROM_HERE, handler->rest_, &handler_,
                             PeonHandler::kWorkWork, handler);
    }

    void MoreWork(WorkWorkData *handler)
    {
        handler->working_ = false;
        // |handler| is deleted in |OnMessage|
        /* if (thread_->IsCurrent()) {
            // cannot |Clear| if more than one |WorkWorkData|
            // thread_->Clear(&handler_, PeonHandler::kWorkWork);
        } else {
            // should wait until |OnMessage| done
            while (!handler->quit_) {
                rtc::Thread::Current()->SleepMs(handler->rest_);
            }
        }
        delete handler; */
    }

private:
    class WorkData : public rtc::MessageData
    {
    public:
        WorkData(Peon *parent, int oper, void *data, int *result)
            : oper_(oper),
              data_(data),
              result_(result),
              parent_(parent)
        {
            if (result_ != nullptr) {
                ; // do nothing
            } else {
                data_ = parent_->DataDup(oper_, data_);
            }
        }
        ~WorkData()
        {
            if (result_ != nullptr) {
                ; // do nothing
            } else {
                // should be here, otherwise memleak will occur when |Stop|
                parent_->DataFree(oper_, data_);
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

#include <string.h>

class Service : public Peon
{
public:
    Service(rtc::Thread *thread)
        : Peon(thread) {}
    virtual ~Service() {}

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

    void *DataDup(int oper, void *data)
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
        std::cout << "DataDup Service: " << oper
                  << ", size: " << size << ", data: " << data_tmp << std::endl;
        return data_tmp;
    }

    void DataFree(int oper, void *data)
    {
        std::cout << "DataFree Service: " << oper << ", data: " << data << std::endl;
        free(data);
    }

    int Process(int oper, void *data)
    {
        std::cout << "Process Service: " << oper << std::endl;

        switch (oper) {
            case kHeng: {
                HengData *data_tmp = (HengData *)data;
                std::cout << "Heng: " << data_tmp->heng_ << std::endl;
                HengHaData count;
                count.hengha_ = 0;
                handler_ = ReadyToWork(kHengHa, &count, 200);
                break;
            }
            case kHa: {
                HaData *data_tmp = (HaData *)data;
                std::cout << "Ha: " << data_tmp->ha_ << std::endl;
                MoreWork(handler_);
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

    void Report(int oper, void *data, int result)
    {
        (void)oper;
        (void)data;
        printf("Report Service: %#x\n", result);
    }

private:
    WorkWorkData *handler_;
};

class Adapter : public Peon
{
public:
    Adapter(rtc::Thread *thread, Service *service)
        : Peon(thread),
          service_(service) {}
    virtual ~Adapter() {}

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

    void *DataDup(int oper, void *data)
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
        std::cout << "DataDup Adapter: " << oper
                  << ", size: " << size << ", data: " << data_tmp << std::endl;
        return data_tmp;
    }

    void DataFree(int oper, void *data)
    {
        std::cout << "DataFree Adapter: " << oper << ", data: " << data << std::endl;
        free(data);
    }

    int Process(int oper, void *data)
    {
        std::cout << "Process Adapter: " << oper << std::endl;

        switch (oper) {
            case kHengHeng: {
                HengHengData *data_src = (HengHengData *)data;
                Service::HengData *data_dst = new Service::HengData;
                data_dst->heng_ = data_src->hengheng_;
                service_->SyncCall(Service::kHeng, data_dst);
                delete data_dst;
                break;
            }
            case kHaHa: {
                HaHaData *data_src = (HaHaData *)data;
                Service::HaData *data_dst = new Service::HaData;
                data_dst->ha_ = data_src->haha_;
                service_->SyncCall(Service::kHa, data);
                delete data_dst;
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
        // bug: data is changed when using |std::hex|, for it changes all numerics to hex after using it
        // std::cout << "Report Adapter: " << std::hex << result << std::endl;
        printf("Report Adapter: %#x\n", result);
    }

    // error: marked ‘override’, but does not override
    int SyncCall(int oper, void *data) /* override */
    {
        return Process(oper, data);
    }

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
        thread_->Stop();
        // will cause segment fault if |thread_| is deleted after |service_|
        // once |service_| is deleted, the |DataFree| funcs and |parent_| in |Peon| will be *Wild*
        // thus segment fault occurs when |WorkWorkData| dtor
        thread_ = nullptr; // delete |std::unique_ptr|
        delete adapter_;
        delete service_;
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
    *(int *)data = 1024;
    project1.adapter_->AsyncMsg(Adapter::kHengHeng, data);
    sleep(1);
    project1.adapter_->SyncCall(Adapter::kHaHa, data);
    free(data);

    rtc::ThreadManager::Instance()->UnwrapCurrentThread();
    // sleep(1);

    return 0;
}
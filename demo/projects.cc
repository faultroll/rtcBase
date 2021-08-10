
#include <iostream>
#include "service.h"
#include "adapter.h"

// - |project|--|adapter|--|service|
//   - 1 |project| 1 |adapter|
//     |project| should be singleton, same |project| means same api
//     |adapter| is needed because of |AsyncMsg| and different api
//     1 |project| may have m |adapter| instance, they(also |service|) should be compiled as needed
//   - 1 |adapter| m |service|
//     1 |service| and 1adapter instance in 1 thread
//     |adapter| should be relative with api(project), while |service| shouldn't
//     |service| should be singleton, different |service| is isolate

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
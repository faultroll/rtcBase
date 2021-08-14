#ifndef ADAPTER_H_
#define ADAPTER_H_

#include "peon.h"

enum Adapters {
    kAAA,
    kBBB,
};

class AdapterAAA : public Peon
{
public:
    AdapterAAA(rtc::Thread *thread, ServiceAlpha *service)
        : Peon(kAAA, thread),
          service_(service) {
              SetRespFunction(this, ReportFunction);
          }
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

    const OperMap *OperFindFunction(int oper)
    {
        static constexpr OperMap oper_map[] = {
            {kHengHeng, sizeof(HengHengData), 0, ProcessFunction},
            {kHaHa, sizeof(HaHaData), 0, ProcessFunction},
        };
        const int len = (int)(sizeof(oper_map) / sizeof(oper_map[0]));
        int i;
        for (i = 0; i < len; i++)
            if (oper_map[i].oper_ == oper)
                break;
        if (i < len)
            return &oper_map[i];
        else
            return nullptr;
    }

    static int ProcessFunction(void *handle, int oper, void *data, void *data_resp)
    {
        AdapterAAA *pthis = (AdapterAAA *)handle;
        std::cout << "ProcessFunction Adapter" << pthis->type_ << ": "
                  << oper << std::endl;

        // if the api is like SendMsg(MsgId msg_id, OperType oper_type, void *data);
        // you should construct you own |data| (you *CANNOT* avoid it)
        // eg. struct Msg {MsgId msg_id_; void *data_};
        // if you don't want to use default |subproc_funciton_|
        // you can pass it through |data|
        // eg.  struct Msg *msg = (struct Msg *)data;
        //      if (msg->subproc_funciton_ != nullptr)
        //          msg->subproc_funciton_(data, data_resp);
        //      else
        //          switch(oper) ... // default |subproc_function_|

        switch (oper) {
            case kHengHeng: {
                HengHengData *data_src = (HengHengData *)data;
                ServiceAlpha::HengData *data_dst = new ServiceAlpha::HengData;
                data_dst->heng_ = data_src->hengheng_;
                pthis->service_->SyncCall(ServiceAlpha::kHeng, data_dst, nullptr);
                delete data_dst;
                break;
            }
            case kHaHa: {
                HaHaData *data_src = (HaHaData *)data;
                ServiceAlpha::HaData *data_dst = new ServiceAlpha::HaData;
                data_dst->ha_ = data_src->haha_;
                pthis->service_->SyncCall(ServiceAlpha::kHa, data, nullptr);
                delete data_dst;
                break;
            }
            default: {
                break;
            }
        }

        return 0xdeadbeaf;
        (void)data_resp;
    }

    static void ReportFunction(void *handle, int oper, int result, void *data_resp)
    {
        AdapterAAA *pthis = (AdapterAAA *)handle;
        // bug: data is changed when using |std::hex|, for it changes all numerics to hex after using it
        // std::cout << "ReportFunction Adapter: " << std::hex << result << std::endl;
        printf("ReportFunction Adapter%d: %#x\n", pthis->type_, result);
        (void)oper;
        (void)data_resp;
    }

    // error: marked ‘override’, but does not override
    int SyncCall(int oper, void *data) /* override */
    {
        return ProcessFunction(this, oper, data, nullptr);
    }

private:
    ServiceAlpha *service_;
};

#endif
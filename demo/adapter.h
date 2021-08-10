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

    void DataDupFunction(int oper, PeonData *data)
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
        memmove(data_tmp, data->data_process_, size);
        std::cout << "DataDupFunction Adapter" << type_ << ": "
                  << oper << ", size: " << size << ", data: " << data_tmp << std::endl;
        data->data_process_ = data_tmp;
        data->data_report_ = nullptr;
    }

    void DataFreeFunction(int oper, PeonData *data)
    {
        std::cout << "DataFreeFunction Adapter" << type_ << ": "
                  << oper << ", data: " << data->data_process_ << std::endl;
        free(data->data_process_);
    }

    int ProcessFunction(int oper, PeonData *data)
    {
        std::cout << "ProcessFunction Adapter" << type_ << ": "
                  << oper << std::endl;

        switch (oper) {
            case kHengHeng: {
                HengHengData *data_src = (HengHengData *)data->data_process_;
                PeonData data_pack;
                ServiceAlpha::HengData *data_dst = new ServiceAlpha::HengData;
                data_dst->heng_ = data_src->hengheng_;
                data_pack.data_process_ = data_dst;
                data_pack.data_report_ = nullptr;
                service_->SyncCall(ServiceAlpha::kHeng, data_pack);
                delete data_dst;
                break;
            }
            case kHaHa: {
                HaHaData *data_src = (HaHaData *)data->data_process_;
                PeonData data_pack;
                ServiceAlpha::HaData *data_dst = new ServiceAlpha::HaData;
                data_dst->ha_ = data_src->haha_;
                data_pack.data_process_ = data_src;
                data_pack.data_report_ = nullptr;
                service_->SyncCall(ServiceAlpha::kHa, data_pack);
                delete data_dst;
                break;
            }
            default: {
                break;
            }
        }

        return 0xdeadbeaf;
    }

    /* void ReportFunction(int oper, PeonData *data, int result)
    {
        (void)oper;
        (void)data;
        // bug: data is changed when using |std::hex|, for it changes all numerics to hex after using it
        // std::cout << "ReportFunction Adapter: " << std::hex << result << std::endl;
        printf("ReportFunction Adapter%d: %#x\n", type_, result);
    } */

    // error: marked ‘override’, but does not override
    int SyncCall(int oper, void *data) /* override */
    {
        PeonData data_pack;
        data_pack.data_process_ = data;
        data_pack.data_report_ = nullptr;
        return Peon::SyncCall(oper, data_pack);
    }

    void AsyncMsg(int oper, void *data)
    {
        PeonData data_pack;
        data_pack.data_process_ = data;
        data_pack.data_report_ = nullptr;
        Peon::AsyncMsg(oper, data_pack);
    }

private:
    ServiceAlpha *service_;
};

#endif
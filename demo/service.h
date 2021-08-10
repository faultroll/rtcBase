#ifndef SERVICE_H_
#define SERVICE_H_

#include <string.h>
#include "peon.h"

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

#endif
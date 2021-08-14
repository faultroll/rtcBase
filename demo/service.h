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
    /* ServiceAlpha *Instance(rtc::Thread *thread)
    {
        static ServiceAlpha *const service = new ServiceAlpha(thread);
        return service;
    } */

    ServiceAlpha(rtc::Thread *thread)
        : Peon(kAlpha, thread)
    {
        SetRespFunction(this, ReportFunction);
    }
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

    const OperMap *OperFindFunction(int oper)
    {
        static constexpr OperMap oper_map[] = {
            {kHeng, sizeof(HengData), 0, ProcessFunction},
            {kHa, sizeof(HaData), 0, ProcessFunction},
            {kHengHa, sizeof(HengHaData), 0, ProcessFunction},
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
        ServiceAlpha *pthis = (ServiceAlpha *)handle;
        std::cout << "ProcessFunction Service" << pthis->type_ << ": "
                  << oper << std::endl;

        switch (oper) {
            case kHeng: {
                HengData *data_tmp = (HengData *)data;
                std::cout << "Heng: " << data_tmp->heng_ << std::endl;
                HengHaData count;
                count.hengha_ = 0;
                pthis->handle_ = pthis->EnterCycle(kHengHa, &count, 200);
                break;
            }
            case kHa: {
                HaData *data_tmp = (HaData *)data;
                std::cout << "Ha: " << data_tmp->ha_ << std::endl;
                pthis->ExitCycle(pthis->handle_);
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
        (void)data_resp;
    }

    static void ReportFunction(void *handle, int oper, int result, void *data_resp)
    {
        ServiceAlpha *pthis = (ServiceAlpha *)handle;
        printf("ReportFunction Service%d: %#x\n", pthis->type_, result);
        (void)oper;
        (void)data_resp;
    }

private:
    AutoCycleData *handle_;
};

#endif
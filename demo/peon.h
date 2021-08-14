#ifndef PEON_H_
#define PEON_H_

#include <string.h>
#include "rtc_base/thread.h"

class Peon // can cascade
{
public:
    Peon(int type, rtc::Thread *thread)
        : type_(type),
          thread_(thread),
          handler_(this),
          resp_handle_(nullptr),
          resp_function_(nullptr) {}
    virtual ~Peon() {}

    // should be private
    struct Result {
        int code_;
        void *data_;
    };

    /* virtual */ void AsyncMsg(int oper, void *data)
    {
        OnePushData *handle = new OnePushData(this, oper, data, nullptr);
        thread_->Post(RTC_FROM_HERE, &handler_,
                      PeonHandler::kOnePush, handle);
    }

    /* virtual */ int SyncCall(int oper, void *data, void *data_resp)
    {
        Result result;
        result.data_ = data_resp;

        OnePushData *handle = new OnePushData(this, oper, data, &result);
        thread_->Send(RTC_FROM_HERE, &handler_,
                      PeonHandler::kOnePush, handle);

        return result.code_;
    }

    // from top to buttom, using virtual function
    typedef int (*ProcFunction)(void *handle, int oper, void *data, void *data_resp);
    struct OperMap { // no need to use `typedef struct OperMap_ { ... } OperMap` in |__cplusplus|
        // key
        int oper_;
        // val
        size_t size_;
        size_t resp_size_;
        ProcFunction proc_function_;
    };
    virtual const OperMap *OperFindFunction(int oper) = 0;
    // from buttom to top, using callback function
    typedef void (*RespFunction)(void *handle, int oper, int result, void *data_resp);

    int SetRespFunction(void *resp_handle, RespFunction resp_function)
    {
        resp_handle_ = resp_handle;
        resp_function_ = resp_function;
        /* if (nullptr == resp_function)
            return -1;
        else */
        return 0;
    }

    // should be private
    class AutoCycleData : public rtc::MessageData
    {
    public:
        AutoCycleData(Peon *parent, int oper, void *data, int interval)
            : parent_(parent),
              oper_(oper),
              cycling_(true),
              interval_(interval)
              // quit_(false)
        {
            const OperMap *map = parent_->OperFindFunction(oper_);
            if (map != nullptr) {
                size_t size;
                size = map->size_;
                if (data != nullptr
                    && size != 0) {
                    data_ = malloc(size);
                    memmove(data_, data, size);
                } else {
                    data_ = nullptr;
                }
                proc_function_ = map->proc_function_;
            } else {
                // not support oper
                data_ = nullptr;
                proc_function_ = nullptr;
            }
        }
        ~AutoCycleData()
        {
            free(data_); // free nullptr is acceptable
        }

        // private:
        Peon *parent_;
        int oper_;
        void *data_;
        bool cycling_;
        int interval_;
        // bool quit_;
        ProcFunction proc_function_;
    };

    // TODO what if cycle msg need result? |data_resp_| seems not working here
    AutoCycleData *EnterCycle(int oper, void *data, int interval)
    {
        AutoCycleData *handle = new AutoCycleData(this, oper, data, interval);
        thread_->Post(RTC_FROM_HERE, &handler_,
                      PeonHandler::kAutoCycle, handle);
        return handle;
    }

    // should be private
    void ToNextCycle(AutoCycleData *handle)
    {
        thread_->PostDelayed(RTC_FROM_HERE, handle->interval_, &handler_,
                             PeonHandler::kAutoCycle, handle);
    }

    void ExitCycle(AutoCycleData *handle)
    {
        handle->cycling_ = false;
        // |handle| is deleted in |OnMessage|
        /* if (thread_->IsCurrent()) {
            // cannot |Clear| if more than one |AutoCycleData|
            // thread_->Clear(&handler_, PeonHandler::kAutoCycle);
        } else {
            // should wait until |OnMessage| done
            while (!handle->quit_) {
                rtc::Thread::Current()->SleepMs(handle->interval_);
            }
        }
        delete handle; */
    }

    int type_;

private:
    class OnePushData : public rtc::MessageData
    {
    public:
        OnePushData(Peon *parent, int oper, void *data, Result *result)
            : parent_(parent),
              oper_(oper)
        {
            const OperMap *map = parent_->OperFindFunction(oper_);
            if (map != nullptr) {
                if (result_ != nullptr) {
                    // |sync| method
                    // dup |data_|
                    data_ = data; // no need to check |size|
                    // dup |data_resp_|
                    data_resp_ = result->data_;
                    result_ = &result->code_;
                } else {
                    // |async| method
                    size_t size;
                    // dup |data_|
                    size = map->size_;
                    if (data != nullptr
                        && size != 0) {
                        data_ = malloc(size);
                        memmove(data_, data, size);
                    } else {
                        data_ = nullptr; // maybe something wrong
                    }
                    // dup |data_resp_|
                    size = map->resp_size_;
                    if (size != 0) {
                        data_resp_ = malloc(size);
                    } else {
                        data_resp_ = nullptr;
                    }
                    result_ = nullptr;
                }
                proc_function_ = map->proc_function_;
            } else {
                // not support oper
                data_ = nullptr;
                data_resp_ = nullptr;
                result_ = nullptr;
                proc_function_ = nullptr;
            }
        }
        ~OnePushData()
        {
            // should be here, otherwise memleak will occur when |Stop|
            free(data_resp_);
            free(data_);
        }

        // private:
        Peon *parent_;
        int oper_;
        void *data_;
        void *data_resp_;
        int *result_;
        ProcFunction proc_function_;
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
                    OnePushData *handle = (OnePushData *)msg->pdata;
                    int result =
                        handle->proc_function_(handle->parent_, handle->oper_, handle->data_, handle->data_resp_);
                    if (handle->result_ != nullptr) {
                        // sync
                        *handle->result_ = result;
                    } else {
                        // async
                        if (parent_->resp_function_ != nullptr)
                            parent_->resp_function_(parent_->resp_handle_, handle->oper_, result, handle->data_resp_);
                    }
                    delete handle;
                    break;
                }
                case kAutoCycle: {
                    AutoCycleData *handle = (AutoCycleData *)msg->pdata;
                    if (handle->cycling_) {
                        handle->proc_function_(handle->parent_, handle->oper_, handle->data_, nullptr);
                        parent_->ToNextCycle(handle);
                    } else {
                        // handle->quit_ = true;
                        delete handle;
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
    void *resp_handle_;
    RespFunction resp_function_;

};

#endif
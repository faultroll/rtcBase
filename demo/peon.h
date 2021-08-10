#ifndef PEON_H_
#define PEON_H_

#include "rtc_base/thread.h"

class Peon // can cascade
{
public:
    Peon(int type, rtc::Thread *thread)
        : type_(type),
          thread_(thread),
          handler_(this),
          report_handle_(nullptr),
          report_function_(nullptr) {}
    virtual ~Peon() {}

    typedef struct PeonData_ {
        void *data_process_;
        void *data_report_;
    } PeonData;

    /* virtual */ void AsyncMsg(int oper, PeonData data)
    {
        OnePushData *handler = new OnePushData(this, oper, data, nullptr);
        thread_->Post(RTC_FROM_HERE, &handler_,
                      PeonHandler::kOnePush, handler);
    }

    /* virtual */ int SyncCall(int oper, PeonData data)
    {
        int result;

        OnePushData *handler = new OnePushData(this, oper, data, &result);
        thread_->Send(RTC_FROM_HERE, &handler_,
                      PeonHandler::kOnePush, handler);

        return result;
    }

    virtual void DataDupFunction(int oper, PeonData *data) = 0;
    virtual void DataFreeFunction(int oper, PeonData *data) = 0;
    virtual int ProcessFunction(int oper, PeonData *data) = 0;
    typedef void (*ReportFunction)(void *handle, int oper, PeonData *data, int result);

    int SetReportFunction(void *report_handle, ReportFunction report_function)
    {
        report_handle_ = report_handle;
        report_function_ = report_function;

        return 0;
    }

    // should be private
    class AutoCycleData : public rtc::MessageData
    {
    public:
        AutoCycleData(Peon *parent, int oper, PeonData data, int interval)
            : oper_(oper),
              data_(data),
              cycling_(true),
              interval_(interval),
              //   quit_(false),
              parent_(parent)
        {
            parent_->DataDupFunction(oper_, &data_);
        }
        ~AutoCycleData()
        {
            parent_->DataFreeFunction(oper_, &data_);
        }

        int oper_;
        PeonData data_;
        bool cycling_;
        int interval_;
        // bool quit_;

    private:
        Peon *parent_;
    };

    AutoCycleData *EnterCycle(int oper, PeonData data, int interval)
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
        OnePushData(Peon *parent, int oper, PeonData data, int *result)
            : oper_(oper),
              data_(data),
              result_(result),
              parent_(parent)
        {
            /* if (result_ != nullptr) {
                ; // do nothing
            } else */ { // always dup, |data_report_| needs malloc
                parent_->DataDupFunction(oper_, &data_);
            }
        }
        ~OnePushData()
        {
            /* if (result_ != nullptr) {
                ; // do nothing
            } else */ {
                // should be here, otherwise memleak will occur when |Stop|
                parent_->DataFreeFunction(oper_, &data_);
            }
        }

        int oper_;
        PeonData data_;
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
                    int result = parent_->ProcessFunction(handler->oper_, &handler->data_);
                    if (handler->result_ != nullptr) {
                        // sync
                        *handler->result_ = result;
                    } else {
                        // async
                        if (parent_->report_function_ != nullptr)
                            parent_->report_function_(parent_->report_handle_,
                                                      handler->oper_, &handler->data_, result);
                    }
                    delete handler;
                    break;
                }
                case kAutoCycle: {
                    AutoCycleData *handler = (AutoCycleData *)msg->pdata;
                    if (handler->cycling_) {
                        parent_->ProcessFunction(handler->oper_, &handler->data_);
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
    void *report_handle_;
    ReportFunction report_function_;
};

#endif
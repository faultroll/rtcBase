
this project extract thread utils from webrtc m66, with my own understands
- trying to make it more lightweight
- thread in webrtc is messagequeue+null_socketserver+platform_thread, before m85 the messagequeue does sooo much than it should; so trying to use thread after m85(but not directly use it, for *awful* absl deps); thread after m85 is using taskqueue and removes messagequeue(merges into thread), seems much better; but we don't need task_queue, because it's thread, *NOT* threadpool
- trying to use platform_thread in thread; you can override run func in thread, so no need to use platform_thread, use thread instead

TODOs
- threadutils(platform_thread_types): using c11 thrd, also replace funcs in criticalsecition(lock)/event(cond)/platform_thread(thread)
- timeutils add timespec calc(totime/totimespec), thus event&platform_thread_types can using these funcs
- add virtual_socketserver/signalthread to demo

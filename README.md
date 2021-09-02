
this project extract thread utils from webrtc m66, with my own understands
- trying to make it more lightweight; more c like then cxx, remove as most cxx features(auto/functor/...) as I can, for embed use
- trying to use thread after m85(but not directly use it, for *awful* absl deps); thread in webrtc is messagequeue+null_socketserver+platform_thread, before m85 the messagequeue does sooo much than it should; thread after m85 is using taskqueue and removes messagequeue(merges into thread), seems much better; but we don't need task_queue, because it's thread, *NOT* threadpool
- trying to use platform_thread in thread; you can override run func in thread, so no need to use platform_thread, use thread instead

TODOs
- add virtual_socketserver/signalthread to demo
- windows version *NOT* tested (or even compiled)

m66: https://github.com/webrtc-uwp/webrtc
m88: https://github.com/open-webrtc-toolkit/owt-deps-webrtc

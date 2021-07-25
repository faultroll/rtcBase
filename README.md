
this project extract thread utils from webrtc m66, with my own understands

- trying to make it more lightweight
- trying to use thread after m85(but not directly use it, for *awful* absl deps); thread after m85 is using taskqueue and removes messagequeue(merges into thread), seems much better; but we don't need task_queue, because it's thread, *NOT* threadpool
- trying to use platform_thread in thread, not raw pthread


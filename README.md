# Does Linux TCP `send()` return 0 if the other side gracefully closes the connection?

This code tries to repro that claim from [here](https://stackoverflow.com/questions/3081952/with-c-tcp-sockets-can-send-return-zero#comment3186149_3082032).


## Running

```sh
gcc -o server server.c && ./server  # in terminal 1

gcc -o server server.c && ./server  # in terminal 2
```

Output:

```
send() returned: 30
Unexpectedly sent 30 bytes.
```

So far this disproves the claim, unless I did something wrong.


## Background and links

### When can `send()` return 0?

There seems to be no documentation that says when `send()` over TCP can return 0 on Linux.

It is only certain that it can return 0 for 0-count input.

There are directly contradicting StackOverflow answers on it:

* [This](https://stackoverflow.com/questions/8900474/when-will-send-return-less-than-the-length-argument/8900775#8900775) claims full network buffers (e.g. on the other side) can cause return-0. Comments below contradict it.
* [This](https://stackoverflow.com/questions/3081952/with-c-tcp-sockets-can-send-return-zero#comment3186149_30820320) claims that further `shutdown()` from the other side always causes return-0. That's what this repo tests.
* [This](https://stackoverflow.com/questions/33987486/c-tcp-sockets-can-send-return-0-after-using-select/33987541#33987541) claims it can only return 0 for 0-count intput, nothing else.
  * The same user says [here](https://stackoverflow.com/questions/44240934/why-is-there-no-flag-like-msg-waitall-for-send/44241287#comment95918542_44241287) that send-all behaviour is mandated by POSIX.

I pointed out these contradictions [here](https://stackoverflow.com/questions/33987486/c-tcp-sockets-can-send-return-0-after-using-select/33987541#comment140584874_33987541).


## LLM note

Gemini 2.5 Pro supported the claim of send-returns-0 on `shutdown()` when prompted:

> In Linux TCP, in which situations does the `send()` syscall return `0` bytes sent?

My response prompt

> Please write example code that demonstrates the "Peer Has Gracefully Closed the Connection" situation.

produced the source code files; I reviewed them.

In `server.c`, it used `close()` instead of `shutdown(sock_fd, SHUT_RD)`, so I replace that. This did not change the results.

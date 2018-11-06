Lab 6
=====

Submit the solution to [Echo](#echo) tasks according to the
[Submitting instructions](#submitting) before Wednesday Nov 5 23:59:59.


Hello
-----

*Note: this is an example, you should submit the [Echo](#echo) task.*

Write a "hello" TCP server. It will listen for TCP connections on a port given
as its first argument (on any address). For every connection, it will read a
"name" from the client (up to first newline, max 60 bytes) and then send
back the string "Hello `<name>`!\n" (where `<name>` is the name read from client
and `\n` is a newline character).

Example:

Server:

```sh
./hello 12345
```

One client (press ctrl-c or ctrl-d to stop if after the reply)
```sh
nc localhost 12345
YoYo
Hello YoYo!
```

```sh
nc localhost 12345
Alexander the Great
Hello Alexander the Great!
```

Note: one "uncooperative" client (e.g. one that does not send anything when it
connects)should not be able to disrupt the functionality for others, i.e. this
client should not stop others from getting responses:

```sh
(echo -n first; sleep 1000; echo last) | nc -q0 localhost 12345
```

There are two basic ways how to solve this: either using threads or asynchronous
(non-blocking) IO. Threaded code is usually easier to write if the threads don't
interact much  (such as in this case), but can become complex / inefficient if a
lot of synchronization is needed and need to be cleaned up properly.
Asynchronous code is usually a bit harder to write upfront (though this depends
on available libraries / frameworks) but might be more efficient and also easier
to maintain when the software grows.

### Threads

Everytime you `accept` a connection, just start a new thread (function) that
handles the connection (`man pthread_create`). You should `join` threads when
they are finished (`pthread_join`).
See the [examples from lecture](../lectures/06/).

Creating new threads and destroying them are *slightly* expensive operations,
so for performance reasons *thread pools* are used sometimes: when a thread
finishes its task, it isn't destroyed but added to a list of *available* threads
that can be reused for other operations. The code to manage all this however
becomes more complicated (the main thread function must add the thread to a
central list and also check some sort of an inbox (queue) for new tasks, both of
which need synchronization).

For the program to be always responsive to new connections, we still need new
threads for all simultaneous connections, which can result in a large ammounts
of threads (imagine thousands of mobile applications connecting to a server to
receive events that don't happen very often).

#### Cleaning up threads

One problem with threads (that your solution should correctly solve/handle) is
that spawned threads should be correctly cleaned up when they finish.

After a thread finishes, some of the associated "management" information is
still kept in the memory, so other threads can inspect it. This might not be
a problem in an application (process) that will terminate immediately after the
(sub-)threads finish, but can be a big resource/memory problem in applications
that run indefinitely such as a network server.

The operation to correctly release any resources / inspect a thread's exit status
after it finishes is usually called `join` (i.e. threads are "joined", `man 3
pthread_join`) and is normally a blocking operation (if the other thread has
not finished yet, the thread calling `join` on it will block until it
finishes), although there usually are non-blocking variants that can indicate
that the other thread is not finished yet (`man 3 pthread_tryjoin_np`, but this
is linux (GNU) specific) or other ways to check if a thread has finished (`man
3 pthread_kill`, look for `ESRCH` but heed the warning there).

Some of the options to clean up threads (with a blocking `accept` loop) are:

- After each `accept` loop over all the running threads and call
  `pthread_tryjoin` (or `pthread_kill` to check if it is running and
  `pthread_join`).

- Set up a new "cleanup" thread that will somehow get notifications from the
  other threads when they finish, and it will call `pthread_join` on them
  (i.e. you will most probably need a thread safe queue with notifications).

- Use thread pools: have a predefined number of threads that don't finish after
  each request, but can handle multiple requests and are cleaned app when the
  whole process finishes. These must run some sort of a main loop to handle
  multiple requests and some form of communication between threads is needed to
  assign requests to threads. This also limits the number of requests that can
  be handled simultaneously by the size of the thread pool.

### Asynchronous

There are two basic problems to solve when handling asynchronous IO:
handling the asynchronous operations themselves (i.e. `select`/`poll`)
and managing the state of the various "tasks" that we are "juggling" at the same
time.

With threads, all the context (data, state, ...) for one "task" (i.e. handling
one connection) could be stored locally in a function. In an asynchronous
framework we need to pause and resume the tasks depending on the state of the
async operations.

In our case this means that we need to remember a buffer and the size of data it
stores for each connection and we need to keep track of whether we are still
reading the name from the client or are sending the answer and set the read /
write `fdset`s for `select` appropriately.

Note that while this might be relatively easy for this hello task (we first just
read a fixed ammount of data and then write some data), it will be more
complicated for the echo task, which needs to handle reads and writes
simultaneously:

- If there is space in the buffer, we need to watch the socket for reading, and
  read data when available. Once we read some data, the buffer became non-empty
  and we need to trigger the writing side (to write the data out, or to watch
  the socket to see when writing is possible).
- If / once the buffer is full, we need to exclude the socket from
  `select` / `poll`, so the network buffer just fills (otherwise it would just
  notify us all the time, that there is data to be read, but can't read it until
  we have space in the buffer).
- If there is data in the buffer, we need to watch the socket for writing and
  write when possible. Once we write some data, the buffer became not full ;)
  and we need to trigger the reading side (to read data in if already waiting
  or to start watching the socket for incoming data)
- If / once the buffer is empty, we need to stop watching the socket for write,
  because the kernel would just immediately tell us, that we can write.

If you think there is a state machine with four states hiding somewhere behind
this, you might not be wrong...


Echo
------

Implement an echo server, i.e. a server that "echoes" (sends back) everything
the client sends. Implement a program that will listen for TCP connection on a
port given as its first argument (on any address).

Your server should behave correctly even if a client is reading more slowly
then sending data (the server should stop reading data from him until it can
again send more data).

### Testing

You can test your solution with the `nc` (netcat) program by running your server
in one terminal and running nc two or more other terminals:

```sh
# in one terminal

./echo 1234

# in other terminals

nc localhost 1234
```

Everytime you type something into `nc` it should print it back,
all all `nc` sessions should be "responsive".

You can test that your server correctly "echoes" everythin back, by sending a big
random file and verifying that you received it correctly:

```sh
# create a 50MB file with random contents
dd bs=1M count=50 if=/dev/urandom of=big

# send / receive
cat big | nc -q0  localhost 1235 > received

# check that the two files are the same
diff big received
```

As an ultimate test, that your server can handle multiple busy clients in parallel,
you can run this "loop" in one (or multiple ;-) terminal:

```sh
cat /dev/urandom  | nc -q0 localhost 1235 >/dev/null
```

and then run multiple "interactive" clients like in the first example, and these
should be still "responsive".



Submitting
----------

Submit your solution by committing required files (at least `echo.cpp`)
under the directory `l06` and creating a pull request against the `l06` branch.

If you split your solution into multiple files, modify appropriately the
dependencies of the `echo` target in the Makefile

A correctly created pull request should appear in the
[list of PRs for `l06`](https://github.com/pulls?utf8=%E2%9C%93&q=is%3Aopen+is%3Apr+user%3AFMFI-UK-2-AIN-118+base%3Al06).

Lab 8
=====

Submit the solution to [TCP chat](#tcp-chat)
according to the [Submitting instructions](#submitting)
before Wednesday 19.11. 23:59:59.

*There are multiple bonuses listed at the end of this assignment. To receive the
bonus points for them, mention which bonuses you solved and briefly describe how
in the PR comment.*

TCP chat
--------

Implement a threaded TCP chat server. It will accept connections on a TCP port
given as the first commandline argument. Whenever a client sends data to the
server, the server shall send the data to every connected client (including the
one that sent the data).

The server must correctly handle slow clients (i.e. where writing to them might
block) by buffering up to 2MB of data per client. If there is not enough space
in a buffer for a particular client to accomodate all data that was received,
the overflow should be dropped.

### Implementation

You should already have an implementation of `RingBuffer`, `Reader` and `Writer`
classes (copy them over to this assignment).
We are still missing management of the clients.

Note that you might want/need to modify the `Writer`/`Reader` classes and the
`run` method to [make it easier to terminate
it](#removing-and-deleting-clients) and also possibly to include a reference to
the client it belongs to (depending on how you implement removal of clients).

```
┌──────────────────────────────────────┐
│ ClientCollection                     │
├────────────┬────────────┬────────────┤
│            │            │            │
└─────v──────┴─────v──────┴─────v──────┘
      |            |            +------------------------------------+
      +----+       +---------------------+                           |
           |                             |                           |
           v                             v                           v
┌─────────────────────────┐ ┌─────────────────────────┐ ┌─────────────────────────┐
│         Client          │ │         Client          │ │         Client          │
├────────┬────────────────┤ ├────────┬────────────────┤ ├────────┬────────────────┤
│ Reader │     Writer     │ │ Reader │     Writer     │ │ Reader │     Writer     │
│        │                │ │        │                │ │        │                │
│     ┌──┴─┐ ┌────────────┤ │     ┌──┴─┐ ┌────────────┤ │     ┌──┴─┐ ┌────────────┤
│     │ fd │ │ RingBuffer │ │     │ fd │ │ RingBuffer │ │     │ fd │ │ RingBuffer │
└──v──┴────┴─┴────────────┘ └─────┴────┴─┴────────────┘ └─────┴────┴─┴────────────┘
   |               ^                           ^                           ^
   \               |                           |                          /
    `--------------+---------------------------+-------------------------'
```

### Managing clients

We also need to manage the list of all clients with following operations:

- add new client -- called from the main thread (accept);
- iterate over all clients (to call `writer->add`) -- called from any reader
  thread;
- remove a client -- called from either a reader or a writer thread


Some thoughts / observations about this:

- Clients (the structures) need to stay at the same place in memory (they will
  be accessed from the reader/writer threads), so any container that reallocates
  memory (basically anything dynamic) will need to manage pointers (`unique_ptr`
  or `shared_ptr`, see below) to clients.

- It doesn't really have to be a list. A set or a map might be better, depending
  on how we identify the clients (`fd` or pointer to the client, can get
  slightly complicated with smart pointers). The requirements are listed above:
  adding, removing (by some key?) and iterating over the clients.

- Removing clients is a bit tricky: removing them from the collection is easy,
  but the actual client object cannot be freed before both the reader and writer
  thread finish (they might access the object). Using a `shared_ptr` (in
  contrast to a `unique_ptr`) might help here. See last paragraph of
  [Removing and deleting clients](#removing-and-deleting-clients).

#### Synchronization

There are multiple ways to ensure thread safety of these operations.

The easiest one is to lock each operation. This is an acceptable solution for
this assignment.

Although the add and remove operations should be fast (depending on the removal
implementation), the iteration over all clients will take longer and will
actually happen most often (every time we receive data from a client). This
operation is however "read only" (when considering the client collection) and
there is no reason why multiple iteration couldn't take place in parallel (as
long as no add /remove operation takes place).

This is the usual multiple readers / multiple writers problem and can thus be
solved by the usual techniques (i.e. RCU or similar).

A relatively simple way to achieve this is to use `shared_ptr`: the collection
is referenced via a global shared pointer. Whenever a (reader) thread needs to
iterate over the collection, it will take a reference (a copy of the
`shared_ptr`) to the collection and iterate over it.

Whenever a write operation needs to be done (add or remove), the corresponding
thread will again take a reference, make a copy of the data, modify it and then
update the global shared pointer to point to this new copy.

The write operations still need to be completely synchronized, but the read-only
accesses need to synchronize only the taking of the reference with its update (
which can be done with either by using a mutex or the corresponding
[atomic operations on `shared_ptr`](http://en.cppreference.com/w/cpp/memory/shared_ptr/atomic)
).


#### Removing and deleting clients

When a client needs to be deleted, the reader and writer threads must be
terminated before the actual data structures are released from memory. There
are multiple ways to achieve this:

- Cancel (terminate) the thread with `pthread_cancel` and then call
  `pthread_join` to actually wait for it to be cancelled. Cancelling threads can
  get messy if the threads allocate and release resources, but in our case these
  can be managed by the client itself. We still need to make sure that the
  Client object is released only after both threads (reader and writer) really
  terminate.

- Include a mechanism to tell the threads (`run` methods of `Reader` and
  `Writer`) to terminate. Because this must be able to interrupt the `read` or
  `write` methods, the only solution (without resorting to non-blocking IO) is
  to use signals:
  - register a signal handler for a signal (say `SIGUSR1`) that does nothing
    **without** the `SA_RESTART` flag (`man sigaction`). This will interrupt
    the `read`/`write` calls  and they will return `EINTR`,
  - the main loop inside the `run` method should check the `running` flag everytime
    `read`/`write` finishes,
  - to send the signal to the thread, use `pthread_kill`.

In both cases the thread that manages the removal must wait for the threads
to really terminate (`pthread_join`) before deleting the Client object.

There is an alternative approach when using signals to stop the thread: if the
client collection stores `shared_ptr`s to the clients, the reader and writer
threads can also "hold" a `shared_ptr` to their client and "release" it when they
correctly finish. The thread initiating the removal can then just remove the
client from the collection and signal the threads to terminate (the client
itself will be deleted when the last `shared_ptr` goes out of scope).


Bonus: T-1000
-------------

Implement a way for the chat server to be gracefully terminated: if the server
receives a `SIGTERM` signal, it should correctly close all open connections and
the server socket and correctly exit. Note that this includes correctly
terminating all threads etc.

Bonus: Reader synchronization
-----------------------------

Implement the client in a way that multiple reader threads don't block when
iterating over the list of clients (as described under
[synchronization](#synchronization)).

Submitting
----------

Submit your solution by modifying committing required files
under the directory `l08` and creating a pull request against the `l08` branch.

If you split your solution into multiple files, modify appropriately the
dependencies of the `chat` target in the Makefile

A correctly created pull request should appear in the
[list of PRs for `l08`](https://github.com/pulls?utf8=%E2%9C%93&q=is%3Aopen+is%3Apr+user%3AFMFI-UK-2-AIN-118+base%3Al08).

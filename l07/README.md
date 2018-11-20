Lab 7
=====

Submit the solution to both tasks ([Reader](#reader) and [Writer](#writer))
according to the [Submitting instructions](#submitting)
before Wednesday Nov 11 23:59:59.

*These tasks are "preparation" for a threaded "TCP chat" server
that we will implement in a future assignment.*


Reader
------

Implement a "reader" that reads data from a file descriptor until it receives
an EOF, error or is signalled to stop. It should pass any read data to a
callback function
(given as a `std::function<void(const char *data, size_t size)`).
See the requirements in the [`Reader.h`](Reader.h) header.

When the `quit` method is called (presumably from a different thread), the `run`
method should finish immediately after the currently running `read` invocation (if any)
finishes (and should not call the callback anymore).

### synchronization

To control the running time of the `run` function, a loop like this can be used:

```c++
class Reader {
  bool running = true;
  void run()
  {
    while (running)
    {
      /* running is not changed here */
    }
  }
  void stop() { running = false; } // run from a different thread
};
```

Such program might seem to work fine, but there are some problems with it.
By default the compiler assumes that variables are not changed from other
threads and could "optimize" the while loop to just `while(true)`. This will
usually be prevented if that block of code calls a function that the compiler
might not be able to reason about (because it doesn't see it's implementation,
i.e. the `write` function from the standard library).

Another problem is that the C++ standard states that the "data race"
(unsynchronized access to the same object from more threads, where at least one
access is a write) is *undefined behaviour*.

The correct way to handle this is to used an atomic variable:

```
std::atomic<bool> running = true;
```

Writer
------

Implement a "writer" that can be used to write all the data coming into a
ring buffer into a file descriptor. See the requirements in the
[`Writer.h`](Writer.h) header.

Similar to the reader, the `run` methods should run until an error is encountered
or the `quit` method is called. If `run` is blocked inside a `write` call at the time,
it should finish immediately after the `write` call finishes... If it is blocked
on the wait condition, it should be woken up.

**Note: for simplicity the examples under the [run](#run) section don't include the required additional
handling of any `running` flag (except for the `while (running)`). You must check it immediately after
waking up from the condition wait or after write finishes.**

### RingBuffer

Use your `RingBuffer` implementation from l05 `;-)`.

Note: because we did not implement a copy / move assignment operator for
`RingBuffer` (and also didn't define a default constructor), it must be
initialized inside an initializer list:

```c++
Writer::Writer(int fd, size_t bufSize)
	: fd(fd)
	, rb(bufSize)
{
/*...*/
}
```

### Synchronization

To coordinate the `add` and `run` methods we will need two synchronization
primitives: a mutex and a wait condition
([`man pthread_mutex_init`](http://manpages.ubuntu.com/manpages/cosmic/man3/pthread_mutex_init.3.html)
and
[`man pthread_cond_init`](http://manpages.ubuntu.com/manpages/cosmic/man3/pthread_cond_init.3.html)).

```c++
class Writer {
private:
	pthread_mutex_t mutex;
	pthread_cond_t cond;
/*...*/
};
```

Both need to be initialized (in the `Writer` constructor):

```c++
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);
```

Instead of the `NULL` parameter we can pass a pointer to a struct that can be
used to customize the behaviour of the mutex / condition. If we are OK with the
default behaviour (as in this case), we can also initialize the variables by
assigning the special values `PTHREAD_MUTEX_INITIALIZER` and
`PTHREAD_COND_INITIALIZER`, for example in an initializer of a C++ class:

```c++
Writer::Writer(int fd, size_t bufSize)
	: fd(fd)
	, rb(bufSize)
	, mutex(PTHREAD_MUTEX_INITIALIZER)
	, cond(PTHREAD_COND_INITIALIZER)
{
}
```

#### Locking/unlocking in C++

Because calling `pthread_mutex_lock` and `pthread_mutex_unlock` can be "fragile"
(and actually unusable if we use exceptions), we will first make a helper
[RAI](http://en.cppreference.com/w/cpp/language/raii) class to manage the lock.
It will lock the mutex in it's constructor and "automatically" unlock it in its
destructor, i.e. when it goes out of scope:

```c++
class Locker {
private:
	pthread_mutex_t *mutex;
public:
	Locker(pthread_mutex_t *m)
		: mutex(m)
	{ pthread_mutex_lock(mutex); }
	~Locker() { pthread_mutex_unlock(mutex); }
};
```

Note: C++ has it's own classes for managing it's "native" locks/mutexes
([std::lock_guard](https://en.cppreference.com/w/cpp/thread/lock_guard)),
however we are using the "raw" C pthreads locks, so we need our own helper
to call the right methods (alternatively we could make a class wrapper around
the pthread mutex that provides the lock/unlock interface `std::lock_guard` expects).

### add

The `add` method needs to lock the mutex, call add on the buffer and then
use `pthread_cond_signal` to notify the `run` method that there is
new data in the buffer (in case it was waiting because the buffer was empty).

Note that we only really need to call `pthread_cond_signal` when the buffer was
empty, but because its performance impact on linux should be pretty low when
nobody is waiting on the condition, it is most probably OK to call it every time
(it can also actually help to recover from a deadlock if waiting on the
condition isn't implemented properly, see below).

Also note that it doesn't matter  whether we signal the condition while holding
the mutex or not, so the simplest implementation can look like this:

```c++
size_t Writer::add(const char *data, size_t size)
{
	Locker lock(&mutex);
	auto added = rb.add(data, size);
	pthread_cond_signal(&cond);
	return added;
}
```

### run

The run method needs, in a loop, to wait on the condition and write the data.

The `pthread_cond_wait` method needs a condition but also a mutex. This is to
avoid race conditions in a situation like this:

```c++
	if (b.isEmpty()) {
		pthread_cond_wait(&cond)
	}
```

After we checked if the buffer is empty, but before the wait call, another
thread might have called the add method to add data, but the notification
on the condition will thus be lost (we are not yet waiting for it):

```c++
//		Thread A					Thread B
	if (b.isEmpty()) {
							auto added = b.add(data.size);
							pthread_cond_signal(&cond);
		pthread_cond_wait(&cond)
	}
```

With our previous `add` implementation this would mean that we would be waiting
until new data comes (and thus would be one data chunk "late"). However if we
signalled the condition in `add` only when adding to an empty buffer, this
would became a deadlock.

With a lock, we can avoid the race:

```c++
	Locker lock(&mutex);
	if (b.isEmpty()) {
		pthread_cond_wait(&cond, &mutex)
	}
```

Because of this `pthread_cond_wait` always expects a **locked** mutex.
It will atomically unlock it and start waiting and will lock it again when
it finishes waiting.

This is almost correct, except for another problem with wait conditions:
due to the way they are implemented, `pthread_cond_wait` can finish even
when it was not signalled. This is called a
["spurious wakeup"](https://en.wikipedia.org/wiki/Spurious_wakeup)
and can happen for example when the process is interrupted by a signal when
waiting. The correct way is therefore to re-check the associated invariant
and possibly call `pthread_cond_wait` again:

```c++
	Locker lock(&mutex);
	while (b.isEmpty()) {
		pthread_cond_wait(&cond, &mutex)
	}
```

*Note that in our use case this might not actually be a problem: we would most
probably just `take` 0 bytes from the buffer and then attempt a `write` of 0
bytes, which will just immediately return.*

With this our `run` method could look something like this:

```c++
void Writer::run()
{
	char buf [1024];
	while (running)  {
		Locker lock(&mutex);
		while (rb.isEmpty()) {
			pthread_cond_wait(&cond, &mutex);
		}

		// TODO loop until we take everything from the ring buffer
		// (or make buf larger then ring buffer)
		size_t taken = rb.take(buf, sizeof(buf));

		// TODO loop until all `taken` bytes are written
		// and also check for errors!
		ssize_t written = write(fd, buf, taken);
	}
}
```

There are two TODOs in this code. The first one is actually not that important:
if we don't consume everything from the buffer, the next iteration of the
outermost while loop will take care of that (although it will unlock and lock
the mutex). The second TODO is more important: we would be dropping data if
we didn't check if everything was written (and we obviously need to check for
errors!).

There is however a more serious problem with this solution: we are calling the
`write` method while we are holding the lock. This means that while we would be
blocked on the write call, all other threads will also be actually blocked if
they called `add`. (Remember that this is why we are going to use threads for
our TCP chat: so that other threads can continue reading and adding data even if
some clients are slow and `write`s to them block).

We just need to move the write out of the locked section:

```c++
	char buf [1024];
	while (running)  {
		{
			Locker lock(&mutex);
			while (rb.isEmpty()) {
				pthread_cond_wait(&cond, &mutex);
			}

			size_t taken = rb.take(buf, sizeof(buf));
		}

		// TODO loop and check for errors
		ssize_t written = write(fd, buf, taken);
	}
```

After implementing the TODO, this would be a correct solution, although a bit
inefficient, because we are moving the data around a lot.

With the interface for ring buffer that we have, we can make it a bit more
efficient: we can use the `front` method of the `RingBuffer` to get a pointer to
the data (and also the size of data that is available). **Because there will be
only one thread running `Writer::run`**, we can then call write on that data
even without locking (`add`-ing will not overwrite our data, and nobody else
will be removing the data). After `write` finishes, we can just `remove` the
appropriate number of bytes from the `RingBuffer`:

```c++
void Writer::run()
{
	while (running)  {
		RingBuffer::CArrayRef front{nullptr,0};
		{
			Locker lock(&mutex);
			while (rb.isEmpty()) {
				pthread_cond_wait(&cond, &mutex);
			}
			front = rb.front();
		}
		// Even if other threads add new data to the ringbuffer,
		// `data` and `size` will be still valid if we have only
		// one "consumer" thread.
		auto written = write(fd, front.data, front.size);
		if (written == -1) { return; }
		{
			Locker lock(&mutex); // Q: do we need this? A: depends on the impl. of remove
			rb.remove(written);
		}
	}
}
```

Couple of notes on this solution:

- There's an important assumption that the information provided by `RingBuffer::front`
  (pointer to the start of data and size) will remain valid (start of data,
  size or the data itself in that location does not change) until we eventually
  call `RingBuffer::remove` (after a possibly long blocking `write`). This
  means that any `RingBuffer:add` invocations from other threads must not
  change that initial portion of the `RingBuffer`'s data (this is usually true
  for any sane implementation of a ring buffer `:-)`). Of course there also must
  not be any other invocations of methods, that would modify it, such as
  another parallel `remove`/`take` call (but we are going to take data
  out of the ring buffer only in this one `Writer` thread, yes?).

- We are locking the `remove` operation, because it might clash with `add`
  operations from other threads. It is however possible to implement `remove` in
  such a way, that it does not need to be locked.

- If the `RingBuffer` is "wrapped around", `front` will give us only part of the
  data and we will write the second part in the next iteration of the `while`
  cycle. The previous implementation issued only a single write at the cost of
  copying the data into a second buffer (assuming the buffer was large enough).
  Also not writing the whole buffer (i.e. having the `write` in a loop) is OK
  this time (albeit less efficient because it will do more locking/unlocking)
  because we remove just the amount of data that was actually written from the
  buffer.

Submitting
----------

Submit your solution by modifying committing required files
under the directory `l07` and creating a pull request against the `l07` branch.

If you split your solution into multiple files, modify appropriately the
dependencies of the `echo` target in the Makefile

A correctly created pull request should appear in the
[list of PRs for `l07`](https://github.com/pulls?utf8=%E2%9C%93&q=is%3Aopen+is%3Apr+user%3AFMFI-UK-2-AIN-118+base%3Al07).

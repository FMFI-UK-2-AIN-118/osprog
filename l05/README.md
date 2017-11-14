Lab 5
=====

Submit the solution to the [ring buffer](#ringbuffer) task according to the
[Submitting instructions](#submitting) before Wednesday Oct 29 23:59:59.

RingBuffer
----------

Implement a [ring buffer](https://en.wikipedia.org/wiki/Circular_buffer)
(also known as a circular buffer) as `RingBuffer` class.
It should support all the operations defined in the
[`RingBuffer.h`](RingBuffer.h) header.

Note that methods of this class are not supposed to be thread safe.  Instead
users of this class must ensure any required synchronization. This allows us
to make better use of the RingBuffer's api, such as using the `front` and
`remove` methods when we know that at most one "consumer" thread will be
running.

There are two APIs to use this class:

- a "safer" interface that copies data but doesn't expose the buffer's internals
  (`add` and `take`)
- a more complex but also more efficient interface, where we get pointers (and
  size) to either the first "part" of the buffer or the space where further
  data should be placed ('front` and `next`) and accompanying methods to signal
  that data was added or removed (`added` and `remove`).

The first method usually means more copy operations: in a "copy" operation we
`read` from the kernel to a temporary buffer (first copy), then call
`RingBuffer::add` on it (second copy). On the other side we call
`RingBuffer::take` (third copy) to a temporary buffer and then call `write`
on it (fourth copy). The methods however hide all the internals of the ring
buffer and we could even make them thread safe by adding locking inside them.

The second interface can be more efficient, but exposes the "internals" of the
ring buffer and allows the user to get it into an inconsistent state or just
cause a buffer overlow or similar error. It's also not feasible to make it
thread safe, because the operations that need to be tied together are split
between two methods (`front`/`remove` and `next`/`added`).

When used in a "copy" scenario, we can tell `read` to copy data directly to the
ring buffer or pass `write` the data directly:

```c++
auto next = ringBuffer.next();
auto bytesRead = read(fd, next.data, next.size)
if (bytesRead > 0 )
    ringBuffer.added(bytesRead);
```

```c++
auto front = ringBuffer.front();
auto bytesWritten = write(fd, front.data, front.size);
if (bytesWritten > 0)
    ringBuffer.remove(bytesWritten)
```

*Note: the ringbuffer could also expose the two "parts" of its buffer as an
array / vector of buffers for
[vectored I/O (or scatter/gather I/O)](https://en.wikipedia.org/wiki/Vectored_I/O)
to be used with interfaces such as
[`readv`/`writev`](https://man7.org/linux/man-pages/man2/readv.2.html).*

We will use this class in following tasks, so you will have to get it "working"
even if you don't submit it correctly on time.

Submitting
----------

Submit your solution by modifying committing required files
under the directory `l05` and creating a pull request against the `l05` branch.

If you split your solution into multiple files, modify appropriately the
dependencies of the `test` target in the Makefile

A correctly created pull request should appear in the
[list of PRs for `l05`](https://github.com/pulls?utf8=%E2%9C%93&q=is%3Aopen+is%3Apr+user%3AFMFI-UK-2-AIN-118+base%3Al05).

Lab 4
=====

Submit the solution to [Buffer](#buffer) task according to the
[Submitting instructions](#submitting) before Wednesday Oct 22 23:59:59.


CBuffer
-------

*Note: This is an example that is already "solved".*

Implement (in plain C) a refernce counted buffer. The [`cbuffer.h`](cbuffer.h)
header file gives the full interface for it and you should implement the
functions in [`cbuffer.c`](cbuffer.c).  You can modify the header, if you
really need to, as long as the tests correctly compile and run.

A new reference counted buffer is created with the `buffer_new` function. It
returns a pointer that is used as a "handle" for the buffer when invoking other
functions (what it points to depends on your implementation and whoever
uses your library should not make any assupmtions about it).

Any functions / parts of code that want to keep a reference to the buffer will
call `buffer_ref`  on it and then they can safely use it until they call
`buffer_unref`.  Note that `buffer_new` automatically *refs* the buffer and thus
the original creator should also call *unref* to release the buffer/memory when
it doesn't need it anymore.

`buffer_data` returns a pointer to the actuall buffer/data in memory that should
be used when actually working with it (such as passing to `read`/`write` calls
etc.). `buffer_size` returns the size of the buffer.

Calling these functions on invalid handles (e.g. after the reference count went
down to zero) results in *undefined behaviour* (i.e. you don't have to check and
handle such cases in the implementation, it's ok if your code crashes when
called with a wrong handle).

Buffer
------

Implement the class `Buffer` in C++ similar to the previous task, without the
use of `std::shared_ptr`, `std::vector` or similar (i.e. you have to manage the
memory yourselves). There are no new, ref and unfref functions because that
functionality can be handled by the constructor, copy constructor and
destructor. It also has to support a `resize` operation.

The interface is given in [`Buffer.h`](Buffer.h). You should implement it in
[`Buffer.cpp`](Buffer.cpp). You can also make changes to the header (ideally
you should need only to add some private methods/members) as long as all the
tests correctly compile and run.

Look at the documentation comments in [`Buffer.h`](Buffer.h) and the tests in
[`TestBuffer.cpp`](TestBuffer.cpp) for specification of the various methods.

The provided Makefile enables [address sanitizer][google-sanitizers]
(Which should be available in recent [gcc][gcc-asan] and [clang][clang-asan]
compilers. If you have problems building with it, you can comment out the
`-fsanitize=address` lines in the Makefile) which will add instrumentation to
the code to check for usual memory errors (leaks, use after free) and exit your
program with a failure status if a problem is detected (along with giving a
textual description similar to valgrind's memcheck). This means that the tests
will fail if address sanitizer detects problems in your code.

*Note: there is also a `memcheck` target that runs `valgrind`'s memcheck tool on
your solution. It is not run by the automatic tests, because the result of
memcheck can't be (easily) automatically detected. Valgrind's instrumentation
and tests are usually slower than the corresponding compiler tools (if
available) but might do more / better checks. Other compilers might also offer
other sanitizers, such as [memory
sanitizer][google-sanitizers].*


    make memcheck

### C++ details

This section gives a summary of some C++ features you will need for this task.

#### Memory allocation

You can allocate "raw" memory (an array of `char`s) with the `new[]` operator
(which allocates an array of objects). You must also delete it with the `delete[]`
operator (and not the "normal" `delete`).

```c++
char* data = nullptr;
//...
data = new char[100]
//...
delete[] data;
```

To "reallocate" it, you of course need to allocate a new buffer, copy over the data
**and** delete the old buffer.

```c++
char* newData = new char[newCapacity];
std::copy_n(data, std::min(size, newCapacity), newData);
delete[] data;
data = newData;
```

Note: plain C has a `realloc` function which can be more efficient (not copy the
data) if the allocator has enough space available to just resize the buffer in
place (and it will do the copying for you). It is however questionable how often
this is possible (you could write a test program to try out different scenarios
;-). If you would want to use that, you of course need to use `malloc` / `free`
instead of `new` / `delete` (you can of course use them even in C++ code, though
it might be considered "not nice").

Another option for "larger" allocations (at least a virtual memory page) is to
use `mmap` directly, which might again make it possible to do more efficient
re-allocation using the properties of the virtual memory system.

#### Copy constructor / assignment operator

When manually managing resources for a class in C++, there are two special constructors
/ operators of interest (in addition to the "normal" constructor(s)):


- The normal constructor

```c++
Buffer(std::size_t = 0);
```

- A copy constructor that is used when a copy of the Buffer is made
  such as when passing it by-value to a function or explicitly through
  `Buffer secondBuffer(otherBuffer);` or `Buffer secondBuffer = otherBuffer;`.

```c++
Buffer(const Buffer& other);
```

- An assignment operator, that is used when an instance of a Buffer is assigned
  another instance via the assignment operator `=`  such as `bufferOne = bufferTwo;`

```c++
Buffer& Buffer::operator=(const Buffer& other);
```


In our case the copy constructor needs to "adopt" the data from the other buffer
and increase the reference count (which should be decreased and checked in the
destructor).

The assignment operator needs to do the same, but must also correctly "decrease"
the reference count of its previous contents and release the data if needed.
This means that it basically performs the same actions as copy constructor and
destructor, but care must be taken in their ordering.

The usual way to easily (and correctly) implement it is the
[copy and swap idiom](https://www.google.com/search?q=c%2B%2B+copy+and+swap):

```c++
Buffer& Buffer::operator=(Buffer other)
{
	std::swap(this->p_d, other.p_d);
	return *this;
}
```

We take the other buffer "by value", i.e. a new copy is created in the `other` variable
using the implementation in the copy constructor (which should increase the reference count),
we then "swap contents" of this and other, and at the end of the function the "temporary"
`other` object is destroyed, which decreases reference to our old data.

You can learn more if you google for the
[C++ rule of three](https://www.google.com/search?q=c%2B%2B+rule+of+three).

*Note*: since C++11 there are also move constructors and assignments. Search for the
[C++ rule of five](https://www.google.com/search?q=c%2B%2B+rule+of+five). You
don't have to implement these for this assignment (unless you want to).

Submitting
----------

Submit your solution by committing required files ([`Buffer.cpp`](Buffer.cpp))
under the directory `l04` and creating a pull request against the `l04` branch.

A correctly created pull request should appear in the
[list of PRs for `l04`](https://github.com/pulls?utf8=%E2%9C%93&q=is%3Aopen+is%3Apr+user%3AFMFI-UK-2-AIN-118+base%3Al04).

[google-sanitizers]: https://github.com/google/sanitizers/wiki
[gcc-asan]: https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html#index-fsanitize_003daddress
[clang-asan]: https://clang.llvm.org/docs/AddressSanitizer.html

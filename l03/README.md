Lab 3
=====

Submit the solution to [PipeWatch](#pipewatch) task according to the
[Submitting instructions](#submitting) before Wednesday Oct 15 23:59:59.


PipeWatch
---------

Implement a (single threaded) program that watches multiple named pipes for
data: whenever data is available, it reads it from the pipes and writes to
stdout according to the output format below.

## Implementation details

In the following text, to "exit with an error" means to print an error to the
standard error and exit with a non-zero exit status.

Your program will receive names of the pipes to watch as arguments:
- If some of the arguments don't exist, your program must create them (using
  `mkfifo`).
- If some of the arguments refer to existing files that are not pipes, your
  program must exit with an error.
- If opening some of the pipes fails (due to permissions or other problems),
  your program must exit with an error.
- When another program finishes writing to a pipe and closes it, the reader will
  see an `EOF` event (read will return 0). Your program should "watch" the pipes
  indefinitely and thus must reopen the pipe (or use other tricks to keep
  reading from the pipe even when the writers close the pipe / exit).

Whenever there is data available to be read from a pipe, the program must read
it and print the following message to standard output

    <PIPE NAME>: <SIZE> bytes\n

where `<PIPE NAME>` is the name of the pipe as given in the arguments, `<SIZE>`
is the number of bytes that was read and `\n` is the newline character. The size
should be the total number of bytes that was read before the pipe was completely
exhausted (`read` returned -1 with `errno` set to `EAGAIN`), i.e. if your
program did two reads of 1024 bytes and then a final read of 10, your program
should print a single line with a size of 2058.

If the data read contains the string `quit`, the program should exit (this
should be handled before the normal message containing the number of read bytes
is output.)

Note that this string can "span" multiple reads, i.e. you can receive `q` as
the last byte of one read and then `uit` as the first three bytes of a next read
(but you don't have to handle it across `select` calls).

Also data written to the pipes might be "binary" and contain zeroes, so string
functions, such as `strstr`, will not work properly. You could use for example
`memmem(3)` (although it is not a standard POSIX or C function, it's is present
in glibc and also many other implementations).

You can use either C or C++ to implement your program, but you must use the
"raw" functions (`read`,`write`,`open` etc) to interact with the pipes and
`select`, `poll` or `epoll` to handle the non-blocking aspect (`poll` or
`epoll` is recommended). You can use any C/C++ way to write the messages to
standard output and error.

```sh
man 7 fifo
man 7 pipe
man 3 mkfifo
man 2 stat
man 2 fstat
man 3 memmem

man 2 select
man 2 poll
man 7 epoll
```

### Examining files

To check whether a file exists, you can either try to directly open it, or you
can use the `stat` function. To check whether a file is a pipe, you can also
use the `stat` or `fstat` function.

The `stat` function receives a path and a pointer to a special
`struct stat` that it will fill out with information about the file or return an error
if the file doesn't exist. The field `st_mode` of that struct contains information about the
type and mode of the file (as a bitmask). The type can be queried by pre-defined macros such as
`S_ISFIFO`, see `man 2 stat` (or `man 7 inode` on some installations) for more details on the
`st_mode` field and macros.

```c++
struct stat sb;
if (stat(filename, &sb) == -1) {
	if (errno == ENOENT) {
		... // file does not exist
	} else {
		perror("stat");
		exit(EXIT_FAILURE);
	}
} else {
	if (!S_ISFIFO(sb.st_mode))
		... / it's not a fifo
}
```

### Select

*Note: `select` is the "older" interface. It's main disadvantage is that it can
only monitor file descriptors whose value is lower than a certain predetermined
value (`FD_SETSIZE`) and thus might be problematic bigger programs that might
have a lot of open files / connections. While it can be more efficient in
certain situations, `epoll` is also bit more "user friendly".*

*TLDR: just use `epoll` for this assignment. The description here is more for
completeness ;-).*

    man 2 select

`select` allows programs to wait for events on files descriptors: when data
becomes available for reading, when space becomes available for writing and when
special "exceptions" occur.

`select` operates on *file descriptor sets*. There are three different sets we
can ask `select` to monitor: the read, write and exception sets. Any of these
can be passed to `select` as `NULL` meaning we are not interested in monitoring
any file descriptors for this type of activity.

File descriptor sets are defined as variables of type `fd_set` (`sys/types.h`)
and manipulated through various macros: `FD_ZERO` should be used to "clear" the
whole set, `FD_SET` to set a particular file descriptor, `FD_ISSET` to test
whether a file descriptor is set (see the manpage for more details and also for
an example at the end).

To make the `select` implementation more efficient, it's first argument is the
maximum file descriptor number, that is set in any of the sets to watch, plus
one.  The `select` call will check only fds up to this number.

A timeout parameter can also be given to `select`, in which case the call will
end when the timeout expires even if no activity is detected, otherwise `select`
would wait indefinitely.

When `select` returns, it will return the number of file descriptors that have
events pending and will modify the fd sets so that only those file descriptors
will be set. These can be used to check which fds need reading or writing.

```c++
while(running) {
	fd_set readFds; // define the set of fds that will be watched for reading
	int maxFd = 0; // select needs to know the highest fd we use, see manual

	FD_ZERO(&readFds);
	for (/* fd in fds we are interested in reading from */) {
		if (fd + 1 > maxFd) maxFd = fd + 1;
		FD_SET(fd, &readFds); // set the fd
	}

	struct timeval tv; // a timeout value for select
	tv.tv_sec = 10;    // set for 10 seconds
	tv.tv_usec = 0;

	int ret = select(maxFd, &readFds, NULL, NULL, &tv);  // wait...

	if (ret == -1) {
		// error happened...
	}
	else if (ret == 0) {
		// timeout expired without any activity (or we got a signanl)
		// maybe we wanted to update a clock, some output ?
	}
	else {
		// we actually have some data to read
		for (/* fd in fds we are interested in reading from */) {
			if (FD_ISSET(fd, &readFds)) {
				// fd actually has something to read
			}
		}
	}
}
```

Note: the above example uses `NULL` for the `writefds` and `exceptfds`
parameters, because we don't watch any descriptors for writing / exceptions.
The last parameter (`&tv`) could also be `NULL` if we didn't want `select` to
wake up after a timeout.

### Poll

    man 2 poll

The poll system call is very similar to the select call, except that the set of
file descriptors is passed as an array of `struct pollfd` elements. These
describe the file descriptor number (`fd`,) the events we are waiting for
(`events`) and a field where `poll` will mark what events actually happened
(`revents` as in "return events".)

Also the optional timeout is specified as a simple number of milliseconds
(and negative number means no timeout.)


```c++
struct pollFds[numFds];

// fill out the watched file descriptors info
int i = 0;
for (/* fd in fds we are interested in reading from */) {
	pollFds[i].fd = fd;
	pollFds[i].events = POLLIN;
	++i;
}

while(running) {
	int ret = poll(pollFds, numFds, -1);
	if (ret == -1) {
		// error happened...
	}
	else if (ret == 0) {
		// timeout expired without any activity (shouldn't happen
		// when we called poll with timeout == -1) or we got a signal
	}
	else {
		// we actually have some data to read
		for (i = 0; i < numFds; ++i) {
			int fd = pollFds[i].fd;
			if (pollFds[i].revents & POLLIN) {
				// fd actually has something to read
			}
			else if (pollFds[i].revents) { // POLLERR or POLLHUP
				// eof or other error
			}
		}
	}
}

```

### Closed pipes

When a program finishes writing to a pipe and closes it, `select`/`poll` will
notify your program and the `read` call will return zero. What happens
afterwards, if you just continue to monitor it as before, is undefined and can
behave differently on different systems.

The correct thing to do in such  case is to close the file descriptor and re-open
the pipe (which might give you a different file descriptor than the one you had
before.)

An alternative way (though it could be considered a hack) is to also open (in
your program) all the pipes for writing: pipes get to the "end of file" state
only when the last writer closes it. If your program keeps the pipes open for
writing, that won't happen (though you might run into other problems in more
complicated programs.)

Submitting
----------

Submit your solution by committing required files (at least `pipewatch.cpp` or `pipewatch.c`)
under the directory `l03` and creating a pull request against the `l03` branch.

A correctly created pull request should appear in the
[list of PRs for `l03`](https://github.com/pulls?utf8=%E2%9C%93&q=is%3Aopen+is%3Apr+user%3AFMFI-UK-2-AIN-118+base%3Al03).

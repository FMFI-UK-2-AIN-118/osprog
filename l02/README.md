Lab 2
=====

Submit the solution to [CopyCat](#copycat) task according to the
[Submitting instructions](#submitting) before Wednesday Oct 8 23:59:59.


CopyCat
-------

Implement a program that copies data from standard input / input file to
standard output / output file. You must use the `read` and `write` C library
functions (that are *almost* the same as the respective system calls) for any
reading and writing (you can use normal functions like `printf`/`fprintf` for
any error messages).

## Implementation details

Implement a function in [`copy.c`](copy.c) with the signature

```c
ssize_t copyFds(int infd, int outfd);
```

that transfers as much data as possible from file identified by file descriptor
`infd` to file identified by file descriptor `outfd`. Note that both file
descriptors can also refer to special files, devices, pipes, network connections
etc. in addition to normal files.

The function must handle all edge cases and relevant errors (`EINTR`,  see
below). It must return the total number of bytes transferred (written to
`outfd`) or `-1` if an error that can't be handled happens.

If an error happens, it must not change the `errno` variable, so that the
calling code can examine it (this means it should not write any output / call
a function that could modify `errno`, such as `printf`)

Also implement a `copycat` program (inside [`copycat.c`](copycat.c)) with the
following behaviour:

- when invoked without any arguments it copies data from standard input to
  standard output (i.e. file descriptors `0` and `1`).

- when invoked with exactly two arguments it copies data from the file named by
  the first argument to the file named by the second argument. If the output
  file exists, it will be overwritten. If any argument is `-` then stdin /
  stdout will be used instead of that file respectively.

The program must correctly handle any errors that occur when opening the files
or when transferring data and also close any file descriptors (files) it opens.
Printing an error to stderr and exiting with a failure status is an acceptable
way to handle errors `:-)`.

*Note: although closing the files in this case is not really needed, because they
will be closed and cleaned up  correctly immediately when the process exists,
you are required to handle it correctly in this task.*

The following sections contain some hints about the implementation.

## Documentation

On most linux systems you should have manpages available, so for most C functions
and posix / linux system calls you can run `man FUNCTION_NAME`.

Because some function names are the same as shell/user commands, you might need to
specify the *section* of the manual pages where to look (user/shell commands are
in section 1, system calls in  section 2 and standard c functions in section 3):

```sh
man 2 read
man 2 write
man open
man close
man 3 exit
man 3 perror
man errno
man fprintf
```

Note: there are usually also man pages for the standard "headers" listing which
types and methods are defined in them (e.g. `man stdio`).

### Reading manpages

Manpages for syscalls and c library function usually contain these sections:

- SYNOPSIS: gives the declaration(s) for the function(s) so that you know how to
  call it. It also gives the header(s) that need to be included.
- DESCRIPTION: describes how the function works / what it does.
- RETURN VALUE: describes what the function returns and what it means. Most
  standard functions return some non-negative number on success and negative
  number when an error occurs.
- ERRORS: this sections lists all the errors a function can exit with.
  If the function returns an error, it will set the global `errno` variable
  to the number of the error so you can check it.
- SEE ALSO: list of other man pages that might be related. If the function
  doesn't do exactly what you want, it might make sense to check these.

## read and write

The `read` and `write` methods correspond almost directly to the respective
system calls:

```c
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
```

Both read and write take similar parameters:
- a file descriptor on which the read/write operation will be executed,
- a buffer (pointer to memory) where the read data will be stored or from where
  data will be taken when writing,
- a `count` parameter that specifies how many bytes should be read / written.

It is up to the caller of the read method to make sure that there is space for
the `count` bytes in the buffer. Similarly it is the callers responsibility to
ensure that `count` isn't bigger then the buffer size.

There is one important thing to note in the RETURN VALUE section for these
methods: they return the actual number of bytes that was read / written.
*However this can be less then was asked for (by the `count` parameter)!* This
will happen for example if we wanted to read 1024 bytes from a file of a smaller
size, but technically could also happen anytime if the kernel has some data
available right now, but would have to wait to actually get all the bytes you
requested.

Takeaway: if you want to read / write a certain number of bytes, you most
probably need to do it in a cycle checking how many you got so far.

The `read` and `write` calls can also return two specific errors that you most
probably want to handle specially: [EINTR](#eintr) and [EAGAIN](#eagain) (see
below). Note: you should handle `EINTR` in your code, but not `EAGAIN` (we'll
talk more about it later when talking about asynchronous/non-blocking io)..

## Handling errors

Standard library functions usually return a non-negative number on success and
a negative number on error (usually -1). When an error happens, they also set
the global variable `errno` to a number indicating the type of the error
(usually represented by a symbolic constant that you can find in the man page /
documentation).

Let's take opening files as an example: the `open` function has the following
signature according to the man page:
```c
int open(const char *pathname, int flags);
```

The description states (among other things) that at least one of the `flags`
needs to be included (to specify how the file should be open), so we will go
with `O_RDONLY` for just reading.

The return value section states that it returns a new file descriptor if
successful or `-1` if an error occurs.

The errors section gives us the full list of errors that can happen.
A correct program / function should handle all the errors, although usually only
some errors have a well defined "recovery mechanism" (if a file doesn't exist
the program might ask the user to enter another name) and the other errors are
usually handled either by exiting the program prematurely or just reporting
the error to upper layers in case of libraries (note: it is never a good idea to
just abort a program inside a library function `:-)`).

With this information we can create the following snippet for opening a file:

```c
if ((fd = open(filename, O_RDONLY)) == -1) {
	if (errno == EACCES) {
		fprintf(stderr, "You don't have permission to access '%s', filename);
	}
	else if (errno == ...) {
		...
	}
	exit(EXIT_FAILURE);
}
```

Note that if you want just to print a message detailing what kind of error
happened, you can use the `perror` function, that will write the given string
followed by an error description to the standard error output:

```c
if ((fd = open(filename, O_RDONLY)) == -1) {
	perror("Can't open file");
	exit(EXIT_FAILURE);
}
```

*Question*: what is wrong in the following example?

```c
if ((fd = open(filename, O_RDONLY)) == -1) {
	fprintf(stderr, "Error openning file '%s'\n", filename);
	if (errno == EACCESS) {
		fprintf(stderr, "Please fix the permissions");
	}
}
```

### EINTR

`EINTR` is a special error used to indicate that a system call has been
interrupted by a signal: remember that if a signal is delivered to a process,
the kernel will "pause" the execution of the process code, run the signal
handler and then "resume" the process in the place it was paused.

If this happens inside a system call, kernel can't just resume the system call.
It will "finish" the current call with the `EINTR` error, call the signal
handler and resume the process execution. The process is responsible to make
the system call again.

Note that on current linux and libc, these are usually handled "automatically"
by the libc implementation (although you can change that when installing a
signal handler with `sigaction`) but it is a good idea to handle this error
anyway if your program might run on different systems / libc implementations.

### EAGAIN

`EAGAIN` is an "error" that can happen when working with file descriptors in
non-blocking mode (more on that later in semester). If you make a system call on
a non-blocking file descriptor (say read or write) and the call can't be
completed right away (there's no data to read, no place to write data), you will
receive this error and know that you should re-try the operation at a later
time.

**You do not have to / should not handle this error specially in this task,
because the `copyFds` function we implement doesn't know what to do with it.**

It should be up to whoever is calling the `copyFds` function to handle that
and call `copyFds` again when possible... However, there is a "small" problem
with the way our function reports errors that prevents the calling code to
correctly resume copying after a failure...

*Question: what is the problem? What would `copyFds` have to return / what
should its interface look like to allow the caller to resume after an error?*

## Creating files

As seen before, the `open` call takes a filename and flags. Opening a file for
writing doesn't create a new file (if it doesn't exist) by default and instead
you get an error. You can either create the file up front (see `creat(2)`) or
pass also the `O_CREAT` flag to open:

```c
int fd = open("somefile", O_WRONLY | O_CREAT);
```

Note that depending on system/user configuration, the file might be created with
wrong access rights and not be actually writable. To avoid that, you can use an
alternate `open` call syntax
`int open(const char *pathname, int flags, mode_t mode);`
where `mode` is the filesystem "mode" i.e. access permissions. These are usually
specified as octal numbers where each part refers to user, group and other
permissions:

```c
fd = open("outfile", O_WRONLY | O_CREAT, 0644);
```

This will create the file with `644` permissions, meaning read+write (4+2) for
user and read-only (4) for group and others.

## Tracing

System calls that a process makes can be traced by the `strace` utility on
linux. You can try it on your program (if you compile it as static you will see
a bit less "noise" that was made when loading dynamic libraries). We will use
the system `cat` program here (note: the actual trace might look a bit different
on your system):

```sh
$ echo "ahoj" > somefile
$ strace cat somefile

execve("/bin/cat", ["cat", "somefile"], [/* 59 vars */]) = 0
brk(NULL)                               = 0x21b500
...
```

Lets look at the different things that are going on here. The first thing that
happened above was the actual execution of the new program in the process. The
`brk` system call normally expands the memory the process can use, but when
called with 0 (`NULL`) it will just return current address of the programs
address "space".

```
mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7febe9e50000
```

`mmap` calls allow you to map files directly to memory, allocate other parts of
memory or modify those already allocated (such as stack).

```
access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
open("/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
fstat(3, {st_mode=S_IFREG|0644, st_size=373050, ...}) = 0
mmap(NULL, 373050, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7febe9df4000
close(3)                                = 0
open("/lib64/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\260\10\2\0\0\0\0\0"..., 832) = 832
fstat(3, {st_mode=S_IFREG|0755, st_size=1664944, ...}) = 0
mmap(NULL, 3771928, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7febe9895000
mprotect(0x7febe9a25000, 2093056, PROT_NONE) = 0
mmap(0x7febe9c24000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x18f000) = 0x7febe9c24000
mmap(0x7febe9c2a000, 15896, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7febe9c2a000
close(3)                                = 0
```

This next part represents the loading of dynamic libraries. First the linker
configuration file is read, then the libraries (in this case only libc) are
opened, the header is read and various parts of the library are mapped into
memory (and some regions may be marked to not be writable to prevent changing
code etc.)

```
mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7febe9df3000
mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7febe9df2000
mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7febe9df1000
arch_prctl(ARCH_SET_FS, 0x7febe9df2700) = 0
mprotect(0x7febe9c24000, 16384, PROT_READ) = 0
mprotect(0x60b000, 4096, PROT_READ)     = 0
mprotect(0x7febe9e51000, 4096, PROT_READ) = 0
munmap(0x7febe9df4000, 373050)          = 0
```

This is some more memory setup, this time done by the actual libc library.

```
brk(NULL)                               = 0x21b5000
brk(0x21d6000)                          = 0x21d6000
```

Libc again checked where the main program address space is and decided to
increase it by a certain amount, thus effectively allocating memory (normally
for heap).

```
open("/usr/lib64/locale/locale-archive", O_RDONLY|O_CLOEXEC) = 3
fstat(3, {st_mode=S_IFREG|0644, st_size=2682224, ...}) = 0
mmap(NULL, 2682224, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7febe9606000
close(3)                                = 0
```

Loading of translated messages...

Now finally the actual `cat` work begins:

```
fstat(1, {st_mode=S_IFCHR|0620, st_rdev=makedev(136, 14), ...}) = 0
```

`cat` checked what the output file (standard output, which is on file
descriptor 1) actually is (note that the "return value" is actually in the
second argument of the call: `fstat` is given a pointer pointing to a structure
which is filled with the info). It found out that it is a character device with
0620 permissions etc.

```
open("somefile", O_RDONLY)              = 3
fstat(3, {st_mode=S_IFREG|0644, st_size=5, ...}) = 0
```

Opening and checking the input file.

```
fadvise64(3, 0, 0, POSIX_FADV_SEQUENTIAL) = 0
```

Tells the kernel how it will access data (kernel can then optimize things by
doing / disabling readahead etc).

```
mmap(NULL, 139264, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7febe9e2e000
```

Allocated some memory (buffer of size 139264?)

```
read(3, "ahoj\n", 131072)               = 5
```

Actual read of 131072 bytes, but only 5 were really read.

```
write(1, "ahoj\n", 5ahoj
)                   = 5
```

The actual write. The output is a bit intermingled because it was to stdout.

```
read(3, "", 131072)                     = 0
```

Reading again, return value of 0 means there's no more data to read, so `cat`
knows it can finish.


```
munmap(0x7febe9e2e000, 139264)          = 0
```

Released the memory (buffer).

```
close(3)                                = 0
close(1)                                = 0
close(2)                                = 0
```

Closed the open file descriptor. Note that the first one (3) was closed
by cat implementation, the others by libc.

```
exit_group(0)                           = ?
+++ exited with 0 +++
```

Actual exit of the program (most probably called by libc based on the return
value from `main`).

Submitting
----------

Submit your solution by committing required files
([`copy.c`](copy.c)/[`.h`](copy.h) and [`copycat.c`](copycat.c)) under the
directory `l02` and creating a pull request against the `l02` branch.

A correctly created pull request should appear in the
[list of PRs for `l02`](https://github.com/pulls?utf8=%E2%9C%93&q=is%3Aopen+is%3Apr+user%3AFMFI-UK-2-AIN-118+base%3Al02).

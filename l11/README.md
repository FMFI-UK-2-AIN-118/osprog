Lab 11
======

Submit the solution to [Process list](#process-list)
according to the [Submitting instructions](#submitting)
before exam.


*If your solution also solves the [bonus](#bonus-forest), state it clearly
in the pull request message.*

*MacOS: sorry, MacOS doesn't have the /proc filesystem, so it's not possible to
test this task directly. You can use the `-p` option of the example program to
read from a 'fake' directory (there is an exmple one under `data/proc`).*

Process list
-------------

Implement a program that lists currently running processes in a format given
below.

The program should read information from the "numbered" directories in
`/proc/` representing processes (see `man 5 proc`), i.e. `/proc/PID/cmdline` and
`/proc/PID/stat`, where `PID` is a number. You can see the descriptions of the
`cmdline` and `stat` files in the proc manpage.

For each process, its PID should be printed (right aligned to 5 characters,
padded with spaces), followed by the commands name and
its arguments (as given by the `/proc/PID/cmdline` file), all separated by
spaces (don't include a "trailing" space after the last argument). The output
should be sorted by the PID value.

```
    1 init [3]
 1862 xterm -class UXTerm -title uxterm -u8
 1867 bash
 4879 /sbin/agetty 38400 tty1 linux
 4880 /sbin/agetty 38400 tty2 linux
 4881 /sbin/agetty 38400 tty3 linux
 4882 /sbin/agetty 38400 tty4 linux
 4883 /sbin/agetty 38400 tty5 linux
 4884 /sbin/agetty 38400 tty6 linux
 4984 kdeinit5: Running...
 4985 klauncher [kdeinit5] --fd=9

```

You can use `opendir`/`closedir` and `readdir` to read directory entries (see `man 3
opendir`, `man 3 readdir`).

Bonus: ASCII forest
-------------------

Implement a `-f` option for your program that renders the list of programs
as a 'forest' according to their parent/child relationship. See the example
below for the output format. See the `stat` file description (in the manpage for
proc) for parent PIDs.

```
    1 init [3]
 4879 \_ /sbin/agetty 38400 tty1 linux
 4880 \_ /sbin/agetty 38400 tty2 linux
 4881 \_ /sbin/agetty 38400 tty3 linux
 4882 \_ /sbin/agetty 38400 tty4 linux
 4883 \_ /sbin/agetty 38400 tty5 linux
 4884 \_ /sbin/agetty 38400 tty6 linux
 4984 \_ kdeinit5: Running...
 1862 |  \_ xterm -class UXTerm -title uxterm -u8
 1867 |     \_ bash
 4985 \_ klauncher [kdeinit5] --fd=9
```

Submitting
----------

Submit your solution by modifying and committing required files (`procs.cpp`)
under the directory `l11` and creating a pull request against the `l11` branch.

If you split your solution into multiple files, modify appropriately the
dependencies of the `procs` target in the Makefile

A correctly created pull request should appear in the
[list of PRs for `l11`](https://github.com/pulls?utf8=%E2%9C%93&q=is%3Aopen+is%3Apr+user%3AFMFI-UK-2-AIN-118+base%3Al11).

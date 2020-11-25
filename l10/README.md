Lab 10
=====

Submit the solution to [Overflow](#overflow) task
according to the [Submitting instructions](#submitting)
before exam.


Overflow
---------

The included `pnginfo` program displays some information about PNG files
(dimensions and the contents of
[tEXt sections / chunks](https://www.w3.org/TR/PNG-Chunks.html#C.tEXt)).
It has various problems in its implementation ([1](#notes) and one of them is a
(rather obvious) buffer overflow.

Exploit this overflow to make the program print a text of the form
`Well done <your AIS login>!`. The actual binary has a set of functions, named
after your AIS logins, similar to the `badWolf` function[2](#notes).

To solve this task you need to create and submit a PNG file (that triggers the exploit)
named `explit.png`.  Although there is source code provided to make your
analysis easier, the PNG file must work against the binary included in the repo
(i.e. not against your recompiled version).

Include also a description of how you constructed the file in the commit or pull
request message.

There is a 64 bit (`pnginfo`) linux binary compiled with debug information to
make debugging easier. MacOS user please compile your binary and submit it
together with the solution ;).

Wikipedia has some
[basic information on buffer overflows](https://en.wikipedia.org/wiki/Buffer_overflow)
and some nice writeups can be found here:
https://sploitfun.wordpress.com/2015/06/26/linux-x86-exploit-development-tutorial-series/

Note: the binaries are statically linked, so you don't have to account for ASLR and
are compiled with `-fno-stack-protector`, so you don't have to work around the build int gcc
stack protection `;-)`.

### Analyzing the binary

#### objdump

You can use `objdump -d FILE` to get a disassembly of a program. Handy for
getting addresses of functions even if you don't plan on reading the assembly
(which should not really be needed to solve  this task):

```sh
$ objdump -d pnginfo | grep badWolf
000000000041505d <badWolf>:
$ objdump -d pnginfo32 | grep badWolf
0805bd90 <badWolf>:
```

#### gdb

You can use `gdb` to see what is happening with your program. Start gdb by
running `gdb ./pnginfo` and then the actual program with `run ARGUMENTS` (i.e.
`run gnu-linux.png`). You can also start gdb with `gdb --args ./pnginfo
gnu-linux.png` and then simply call `run`.

```
yoyo@tableta l10 $ gdb --args  ./pnginfo gnu-linux.png
GNU gdb (Gentoo 7.11 vanilla) 7.11
 <snip>
Reading symbols from ./pnginfo...done.
(gdb)
```

You can set breakpoints with `break <source:line or fucntion>`

```
(gdb) break pnginfo.cpp:57
Breakpoint 1 at 0x401645: file pnginfo.cpp, line 57.
(gdb) run
Starting program: /home/yoyo/2017/skola/zima/osprog/osprog/l10/pnginfo gnu-linux.png

Breakpoint 1, PngInfo::PngInfo (this=0x7fffffffdb50, fname=0x7fffffffe02e "gnu-linux.png") at pnginfo.cpp:57
57                      if (!file.read(fileHdr, sizeof(fileHdr)))

```

See current frame information with `info frame`
```
(gdb) info frame
Stack level 0, frame at 0x7fffffffdb40:
 rip = 0x401645 in PngInfo::PngInfo (pnginfo.cpp:57); saved rip = 0x4012a0
 called by frame at 0x7fffffffdbd0
 source language c++.
 Arglist at 0x7fffffffdb30, args: this=0x7fffffffdb50, fname=0x7fffffffe02e "gnu-linux.png"
 Locals at 0x7fffffffdb30, Previous frame's sp is 0x7fffffffdb40
 Saved registers:
  rbx at 0x7fffffffdb20, rbp at 0x7fffffffdb30, r12 at 0x7fffffffdb28, rip at 0x7fffffffdb38
```

Inspect variables with `print`, step through code with `n`:

```
(gdb) print &fileHdr
$1 = (char (*)[8]) 0x7fffffffdab0
(gdb) print fileHdr
$2 = "\000\000\000\000\000\000\000"
(gdb) n
60                      if (strncmp(fileHdr, (const char*)pngHdr, 8))
(gdb) print fileHdr
$3 = "\211PNG\r\n\032\n"

```

You can also examine memory locations with `x`, for example `x/16xw ADDR` will
print 16 "words" (4 byte values) in hex starting at ADDR (you can use a number
directly or a pointer expression such as `&fileHdr`). You can use this to
examine the stack for the current frame (the addresses from the `info frame`
output).


### Hex editor

You can use either a hex editor or a tool like `xxd` (mentioned below) to craft
the malicious PNG file. Some linux hex editors include:

- okteta (part of KDE)

- the viewer of Midnight Commander (`mc`). Either invoke it in `mc` by pressing
  `F3` on a file, or run it standalone as `mcview FILE`. In both cases it will
  most probably detect that you are viewing a picture and thus display only
  information about it. Press `F8` to swithc to "raw" mode and then `F4` to
  switch to hex mode. Press `F2` to actually start editing (changing the file)
  and `F6` to save it (it will also ask you if you try to exit with pending
  changes).


### Notes

1. Among some design problems, there is also a possible memory leak (though not
   that important given the nature of the main function, but it could be a
   problem if PngInfo was part of a library) and the fact that it doesn't [check
   / validate](https://www.w3.org/TR/PNG-Decoders.html#D.Text-chunk-processing)
   and [restrict /
   normalize](https://www.proteansec.com/linux/blast-past-executing-code-terminal-emulators-via-escape-sequences/)
   the information from the PNG file before printing it to your console.
2. Yes, this is supposed to be rather easy ;). You can try to use other
   techniques to actually execute injected code instead of just executing a
   preexisting function (which however is still pretty powerful in bigger
   programs that might contain some interesting functionality normally not
   accessible / triggered by data).
3. The [`gnu-linux.png`](gnu-linux.png) image was taken from the "Archlinux"
   GRUB theme: https://github.com/Generator/Grub2-themes/

Submitting
----------

Submit your solution by committing required files (`exploit.png`)
under the directory `l10` and creating a pull request against the `l10` branch.

A correctly created pull request should appear in the
[list of PRs for `l10`](https://github.com/pulls?utf8=%E2%9C%93&q=is%3Aopen+is%3Apr+user%3AFMFI-UK-2-AIN-118+base%3Al10).

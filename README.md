# Reproducing a memmove bug in GNU libc

There is a bug in GNU libc's memmove in 32bit binaries when moving memory
across the 2GB boundary. My guess is that there is a sign issue here: the
initial address of the move range is "positive" (top bit not set), the final
address is "negative" (top bit set). 

To test if your system is affected, just type `make`.

Note that this requires a 32-bit compiler and runtime environment to be
installed. This may require installing additional packages if you are on a
64-bit system. On Debian or Ubuntu, you can get the required packages via

    apt install gcc-multilib libc6-dbg

Assuming these requirements are met, you should see output like this on
affected systems:

    allocating 2147483648 = 0x80000000 bytes...
    start = 0x7758f008, end = 0xf758f008
    init memory...
    move memory...
    check memory...
    ABORT: mismatch at offset 698706: expected 1651619703 but got 681191333


You can also pass the option `-m` to `./a.out` to force it to not use
`memmove` but instead some naive hand written code moving the data
byte-by-byte. This then should work on ever system.


I'd be interested in learning more about affected systems. So if you have run
the above command, consider sending me the result, together with the output of
`make report` (which contains the versions of GNU libc, the C compiler, and
your kernel). If you can figure it out, it would also be good to know which
exact `memmove` variant is being used; GNU libc contains multiple, with
various optimizations (e.g. for SSE2, SSE3, AVX, ...) and I suspect that only
some of them are buggy. Unfortunately I don't know of a good way to figure
this out, so far I did it via gdb, by setting a breakpoint on `memmove` and
then single-stepping into it until the actual implementation is reached, such
as `__memmove_sse2_unaligned`, `__memmove_ssse3`, ...

Submit the result as an issue via <https://github.com/fingolfin/memmove-bug/issues>.


## Affected versions

So far, I was able to reproduce the bug with GNU libc 2.21, 2.23, 2.24, 2.25.
In contrast, I could not reproduce it with 2.5, 2.17, 2.19, 2.20

Thus, it seems the issue was introduced in 2.21. Indeed, that version added
__memmove_sse2_unaligned which is what is used in all broken cases, as
far as I can tell.

Looking at the glibc sources for that function, it seems it performs signed
comparisons of address in several places. I added a patch file,
`glibc-memcpy.patch`, which addresses those, and thus *might* fix the issue;
but so far I have not tested it (would have to figure out how to compile and
use my own libc for that).


### Details on affected systems

1. "bruhat" running Ubuntu 16.04.3 LTS (GNU/Linux 4.4.0-91-generic x86_64)
  * GNU C Library (Ubuntu GLIBC 2.23-0ubuntu9) stable release version 2.23, by Roland McGrath et al.
  * gcc (Ubuntu 5.4.0-6ubuntu1~16.04.5) 5.4.0 20160609
  * 4.4.0-91-generic #114-Ubuntu SMP Tue Aug 8 11:56:56 UTC 2017
  * `__memmove_sse2_unaligned`

2. "hirsch" running Debian 9.3
  * GNU C Library (Debian GLIBC 2.24-11+deb9u1) stable release version 2.24, by Roland McGrath et al.
  * gcc (Debian 6.3.0-18) 6.3.0 20170516
  * 4.9.0-3-amd64 #1 SMP Debian 4.9.30-2+deb9u5 (2017-09-19)
  * `__memmove_sse2_unaligned`

3. "murrumesh" running Gentoo (Base System release 2.4.1)
  * GNU C Library (Gentoo 2.25-r9 p12) stable release version 2.25, by Roland McGrath et al.
  * gcc (Gentoo 4.9.4 p1.0, pie-0.6.4) 4.9.4
  * 3.16.5-gentoo #1 SMP Tue Dec 16 15:06:31 CET 2014

4. Fedora 22
  * GNU C Library (GNU libc) stable release version 2.21, by Roland McGrath et al.
  * cc (GCC) 5.3.1 20160406 (Red Hat 5.3.1-6)
  * 4.4.14-200.fc22.x86_64 #1 SMP Fri Jun 24 21:19:33 UTC 2016
  * `memmove == __memmove_sse2_unaligned`

### Details on systems which are not affected

1. "lovelace" running Scientific Linux 7
  * GNU C Library (GNU libc) stable release version 2.17, by Roland McGrath et al.
  * gcc (GCC) 6.3.1 20170216 (Red Hat 6.3.1-3)
  * 3.10.0-693.1.1.el7.x86_64 #1 SMP Tue Aug 15 08:36:44 CDT 2017
  * `memmove == __memmove_ssse3`

2. "seress" running Debian 8.10
  * GNU C Library (Debian GLIBC 2.19-18+deb8u10) stable release version 2.19, by Roland McGrath et al.
  * gcc (Debian 4.9.2-10) 4.9.2
  * 3.16.0-4-amd64 #1 SMP Debian 3.16.39-1+deb8u2 (2017-03-07)
  * `memmove == __memmove_ssse3_rep`

3. "yin"
  * GNU C Library stable release version 2.5, by Roland McGrath et al.
  * gcc (GCC) 4.1.2 20080704 (Red Hat 4.1.2-55)
  * 2.6.18-416.el5xen #1 SMP Fri Oct 28 11:56:28 UTC 2016

4. Fedora 21
  * GNU C Library (GNU libc) stable release version 2.20, by Roland McGrath et al.
  * cc (GCC) 4.9.2 20150212 (Red Hat 4.9.2-6)
  * 4.1.13-100.fc21.x86_64 #1 SMP Tue Nov 10 13:13:20 UTC 2015
  * `memmove == __memmove_ssse3_rep`

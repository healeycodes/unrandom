# 🎲 unrandom
> My blog post: [Making Python Less Random](https://healeycodes.com/making-python-less-random)

<br>

Force `getrandom` syscalls from a process to return zeroes on x86-64 Linux.

Build and run it:

```txt
gcc -o unrandom unrandom.c
./unrandom <pid>
```

Then start a Python REPL:

```txt
>>> import os
>>> os.urandom(8)
b'\x00\x00\x00\x00\x00\x00\x00\x00'
>>> os.urandom(8)
b'\x00\x00\x00\x00\x00\x00\x00\x00'
```

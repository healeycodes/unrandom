# unrandom

Force `getrandom` syscalls from a process to return zeroes on x86-64 Linux.

```
gcc -o unrandom unrandom.c
./unrandom <pid>
```

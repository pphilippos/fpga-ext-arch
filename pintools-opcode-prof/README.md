### Introduction

This directory contains the Pin Tools that were used inside the paper to profile the localities of the opcodes.

These Pin tools are modifications of the original ``source/tools/ManualExamples/pinatrace.cpp``. Thus, familiarity with running that Intel Pin tool would be enough to reproduce our experiments.

Tested with Intel Pin on Arch Linux.

Intel Pin version: ``pin-3.22-98547-g7a303a835-gcc-linux``
Architecture: x86_64

Note that according to the runtime of the benchmark, it can be hundreds of times slower when profiling (e.g. a few days for Geekbench 5 Single-core on AMD Ryzen 7 PRO 4750U).

### Contents

There are two versions:

- ``pinatrace2e.cpp``: Single-threaded version. 

This has the thread safety mechanisms, but it is the next one that was used for the multi-threading experiment in the paper. In contrast to a previous iteration (hence the unused ``Thread`` knob), this observes all threads (useful for avoiding searching for the active thread, e.g. when there is a main threads that spawns others). 

- ``pinatrace2f.cpp``: Multi-threaded version. 

Similar as above but has the distribution information for violin plots. Although multi-threading is studied, this is destined for oversubscription to a single core using ``taskset`` (see below).


### Example runs

- Single-threaded version

Command:

```
bash -c ' /home/philippos/pin-3.22-98547-g7a303a835-gcc-linux/pin -t /home/philippos/pin-3.22-98547-g7a303a835-gcc-linux/source/tools/ManualExamples/obj-intel64/pinatrace2e.so -thread -1 -out summary.txt -- java -Duser.dir=/run/media/philippos/Samsung/.phoronix-test-suite/installed-tests/pts/java-scimark2-1.1.2 jnt.scimark2.commandline' &> stdout.out
```

Example summary from above call:

```
window 32 perc 0.5.25.50.75.95.100: ops 1 6 6 6 9 15 25 ips 1 2 2 2 3 5 10 ads 0 2 3 3 4 9 26 samples 53351
window 64 perc 0.5.25.50.75.95.100: ops 1 6 6 6 12 22 31 ips 1 2 2 2 5 9 15 ads 0 3 4 5 8 14 50 samples 26675
window 128 perc 0.5.25.50.75.95.100: ops 1 6 6 6 16 28 40 ips 1 2 2 2 8 15 23 ads 1 6 7 7 13 22 100 samples 13337
window 256 perc 0.5.25.50.75.95.100: ops 1 6 6 6 19 33 44 ips 1 2 2 2 12 26 45 ads 2 11 12 12 22 34 197 samples 6668
window 512 perc 0.5.25.50.75.95.100: ops 2 6 6 6 27 38 52 ips 1 2 2 2 25 39 89 ads 2 21 22 22 40 55 387 samples 3334
window 1024 perc 0.5.25.50.75.95.100: ops 6 6 6 6 34 41 57 ips 2 2 2 2 39 50 132 ads 9 40 41 42 58 73 507 samples 1667
window 2048 perc 0.5.25.50.75.95.100: ops 6 6 6 6 36 46 71 ips 2 2 2 2 43 79 225 ads 19 48 81 82 86 106 603 samples 833
window 4096 perc 0.5.25.50.75.95.100: ops 6 6 6 6 40 52 74 ips 2 2 2 2 47 118 355 ads 27 81 127 161 162 184 682 samples 416
window 8192 perc 0.5.25.50.75.95.100: ops 6 6 6 6 40 61 83 ips 2 2 2 2 56 186 591 ads 49 136 180 320 323 350 775 samples 208
window 16384 perc 0.5.25.50.75.95.100: ops 6 6 6 6 41 68 95 ips 2 2 2 2 72 321 718 ads 87 209 277 639 644 709 898 samples 104
window 32768 perc 0.5.25.50.75.95.100: ops 6 6 6 6 43 75 99 ips 2 2 2 2 93 550 1235 ads 222 341 459 1276 1287 1392 1634 samples 52
window 65536 perc 0.5.25.50.75.95.100: ops 6 6 6 6 43 81 101 ips 2 2 2 2 117 661 1309 ads 581 581 753 2550 2555 2698 3127 samples 26
window 131072 perc 0.5.25.50.75.95.100: ops 6 6 6 6 43 94 101 ips 2 2 2 2 133 1048 1351 ads 1008 1008 1699 5099 5099 5292 5823 samples 13
window 262144 perc 0.5.25.50.75.95.100: ops 6 6 6 6 43 83 94 ips 2 2 2 2 136 937 1048 ads 2395 2395 2395 8165 10196 10196 10383 samples 6
window 524288 perc 0.5.25.50.75.95.100: ops 6 6 6 6 43 43 102 ips 2 2 2 2 136 136 1580 ads 9980 9980 9980 9980 17185 17185 20390 samples 3
icount 1707237
```

- Multi-threaded version 

Command: 

```
taskset 0x1 bash -c ' /home/philippos/pin-3.22-98547-g7a303a835-gcc-linux/pin -t /home/philippos/pin-3.22-98547-g7a303a835-gcc-linux/source/tools/ManualExamples/obj-intel64/pinatrace2f.so -thread -1 -out summaries/embench8.txt -- embench_allinone/main8 ' &> stdout/embench8.out
```

Example summary from above call:
```
opcodes window 32 key 1 frequency 12
opcodes window 32 key 2 frequency 1814267
opcodes window 32 key 3 frequency 529525
opcodes window 32 key 4 frequency 3226303
opcodes window 32 key 5 frequency 27107883
[...]
opcodes window 32 key 27 frequency 71123
opcodes window 32 key 28 frequency 5470
opcodes window 32 key 29 frequency 9
opcodes window 32 key 30 frequency 1
instr window 32 key 1 frequency 17500240
instr window 32 key 2 frequency 76412604
instr window 32 key 3 frequency 106213314
instr window 32 key 4 frequency 56137863
instr window 32 key 5 frequency 35074761
instr window 32 key 6 frequency 15434111
instr window 32 key 7 frequency 5463151
instr window 32 key 8 frequency 1085524
instr window 32 key 9 frequency 427602
instr window 32 key 10 frequency 6637
instr window 32 key 11 frequency 158
instr window 32 key 12 frequency 12
data window 32 key 0 frequency 16106269
data window 32 key 1 frequency 36553151
data window 32 key 2 frequency 77029831
[...]
data window 32 key 19 frequency 18423
data window 32 key 20 frequency 2
opcodes window 64 key 1 frequency 5
opcodes window 64 key 2 frequency 98712
[...]
```

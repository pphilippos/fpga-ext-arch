
## Softcore framework (Simodense (RV32IM) extended with "F" (RV32IMF) (implemented behaviourually) and "Zicsr". This also targets the faster Verilator rather than the more realistic iVerilog for simulation.

(More documentation pending.)

- If verilator is installed, running ``bash runv.sh`` should be able to run a FreeRTOS experiment that oversubscribes 2 tasks (Embench's ud and wikisort). 
 
- ``-BL1mode=1`` in ``runv.sh`` represents the correct compartmentalisation scenario in the paper (instructions groupped according to their logic similarity). (mode 0 and 2 were later abandoned for brevity).

- ``-DBL1lat=10 -DBL1lathit=1``, for instance, means a 10-cycle bitstream miss latency, and an 1-cycle bitstream miss latency, as described in the paper.

Example run:

```
Simulation starting...
Hello FreeRTOS!
Creating queue...
OK
Create task ud
Create task wikisort
Starting scheduler...
Initialised ud. Warming up 1
Initialised wikisort. Warming up 1
Warmed up ud
Warmed up wikisort
wikisort cycles 56187341 instructions 33251824 [OK?1]
Initialised wikisort. Warming up 1
Warmed up wikisort
wikisort cycles 56185454 instructions 33251193 [OK?1]
Initialised wikisort. Warming up 1
Warmed up wikisort
ud cycles 154795531 instructions 91603887 [OK?1]
Simulation ended...
[philippos@araucaria simodensef]$ 

```

Click [here](https://github.com/pphilippos/simodense) for more information on the original simodense.

Click [here](https://arxiv.org/pdf/2203.10359.pdf) to read the paper.

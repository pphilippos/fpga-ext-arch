## FPGA-extended General Purpose Computer Architecture

### Welcome to the open-source repository for all experiments found in our paper

_Philippos Papaphilippou, Myrtle Shah "FPGA-extended General Purpose Computer Architecture" The 18th International Symposium on Applied Reconfigurable Computing (ARC) 2022_

This is a research paper exploring a novel FPGA-extended computer architecture, where small FPGAs are used to implement instructions inside general-purpose processors (CPUs). Click [here](https://arxiv.org/pdf/2203.10359.pdf) for a preprint.

### Contents

- [simodensef](https://github.com/pphilippos/fpga-ext-arch/tree/main/simodensef): Softcore framework (Simodense (RV32IM) extended with "F" (RV32IMF) (implemented behaviourually) and "Zicsr". This also targets the faster Verilator rather than the more realistic iVerilog for simulation.
- [benchmarks](https://github.com/pphilippos/fpga-ext-arch/tree/main/benchmarks): The ported benchmarks (Embench with F) and operating system (FreeRTOS) etc.
- [nextpnr-model](https://github.com/pphilippos/fpga-ext-arch/tree/main/nextpnr-model): The FPGA architecture model (nextpnr experiment)
- [pintools-opcode-prof](https://github.com/pphilippos/fpga-ext-arch/tree/main/pintools-opcode-prof): Intel PIN experiments for opcode characterisation


### Documentation

This is mostly of research nature, thus, there is minimal documentation currently, but it will be improved eventually. The [paper](https://arxiv.org/pdf/2203.10359.pdf) has important techical information that can help with reproducing results.

As the experiments are based on other tools such as [nextpnr] (https://github.com/YosysHQ/nextpnr), however, it may be easier to become familiar with those first. For the Simodense experiments you can start from the [original repository](https://github.com/pphilippos/simodense), which contains tutorials, video etc. 

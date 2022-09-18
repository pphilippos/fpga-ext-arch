## fpga-ext-arch

### Welcome to the open-source repository for all experiments found in the paper:

_Philippos Papaphilippou, Myrtle Shah "FPGA-extended General Purpose Computer Architecture" The 18th International Symposium on Applied Reconfigurable Computing (ARC) 2022_

This is a research paper exploring a novel FPGA-extended computer architecture, where small FPGAs are used to implement instructions inside general-purpose processors (CPUs). Click [here](https://arxiv.org/pdf/2203.10359.pdf) for a preprint.

The following content placeholders are going to be eventually populated soon:

- Softcore framework (Simodense (RV32IM) extended with "F" (RV32IMF) and "Zicsr")
- The ported benchmarks (Embench with F) and operating system (FreeRTOS)
- [nextpnr-model](https://github.com/pphilippos/fpga-ext-arch/tree/main/nextpnr-model): The FPGA architecture model (nextpnr experiment)
- Intel PIN experiments for opcode characterisation


### Documentation

Note: as this is mostly of research nature, there is minimal documentation currently, but it will be improved eventually. As the experiments are based on other tools such as [nextpnr] (https://github.com/YosysHQ/nextpnr), however, it may be easier to become familiar with those first. For the Simodense experiments you can start from the [main repository](https://github.com/pphilippos/simodense), which contains tutorials, video etc.

## FPGA-extended General Purpose Computer Architecture

### Welcome to the open-source repository for all experiments found in our paper

_Philippos Papaphilippou, Myrtle Shah "FPGA-extended General Purpose Computer Architecture" The 18th International Symposium on Applied Reconfigurable Computing (ARC) 2022_

This is a research paper exploring a novel FPGA-extended computer architecture, where small FPGAs are used to implement instructions inside general-purpose processors (CPUs). Click [here](https://arxiv.org/pdf/2203.10359.pdf) to read the manuscript.

### Contents

- [simodensef](https://github.com/pphilippos/fpga-ext-arch/tree/main/simodensef): Softcore framework (Simodense (RV32IM) extended with "F" (RV32IMF) (implemented behaviourually) and "Zicsr". This also targets the faster Verilator rather than the more realistic iVerilog for simulation.
- [benchmarks](https://github.com/pphilippos/fpga-ext-arch/tree/main/benchmarks): The ported benchmarks (Embench with F) and operating system (FreeRTOS) etc.
- [nextpnr-model](https://github.com/pphilippos/fpga-ext-arch/tree/main/nextpnr-model): The FPGA architecture model (nextpnr experiment)
- [pintools-opcode-prof](https://github.com/pphilippos/fpga-ext-arch/tree/main/pintools-opcode-prof): Intel PIN experiments for opcode characterisation


### Documentation

This is mostly of research nature, thus the [paper](https://arxiv.org/pdf/2203.10359.pdf) has important techical information that can help with understanding the framework and reproducing results.

As the experiments are based on other tools, it is easier to become familiar with those first. For the Simodense experiments you can start from the [original Simodense repository](https://github.com/pphilippos/simodense), which contains tutorials, video etc. For experimenting with the FPGA model see [nextpnr](https://github.com/YosysHQ/nextpnr).

For any questions, please do not hesitate to contact [me](https://philippos.info).

### Paper details

Philippos Papaphilippou, Myrtle Shah "FPGA-extended General Purpose Computer Architecture" The 18th International Symposium on Applied Reconfigurable Computing (ARC) 2022 [pdf](https://arxiv.org/pdf/2203.10359.pdf) [link](https://doi.org/10.1007/978-3-031-19983-7_7) [source](https://github.com/pphilippos/fpga-ext-arch) [program](https://nicsefc.ee.tsinghua.edu.cn/detail.html?id=1030) [slides](https://www.researchgate.net/publication/363652284_FPGA-extended_General_Purpose_Computer_Architecture_slides) [video](https://youtu.be/B-UI6G1Cws8) [bib](http://philippos.info/papers/fpgaext.bib) 

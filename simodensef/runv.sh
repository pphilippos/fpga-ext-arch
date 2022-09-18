#export SYSTEMC_HOME=/home/philippos/Loki/systemc-2.3.1
#export SYSTEMC_INCLUDE=$SYSTEMC_HOME/include
#export SYSTEMC_LIBDIR=$SYSTEMC_HOME/lib/x86_64-linux-gnu
#export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:$SYSTEMC_HOME/lib/x86_64-linux-gnu

#verilator  -cc tester.v --top-module tester -Wno-context  -Wno-fatal -Wno-lint -Wno-style  tester -DBIN="\"benchmarks/embench-iot/bd/src/primecount/primecount\"" #-Wno-all #-Wall -Wno-STMTDLY -Wno-DECLFILENAME -Wno-WIDTH #--sc

verilator  -cc tester.v --top-module tester -Wno-context  -Wno-fatal -Wno-lint -Wno-style  tester -DBL1mode=1 -DBL1lat=10 -DBL1lathit=1 -DBL1ways=4 -DBIN="\"firmware.bin\"" 

make -C obj_dir -f Vtester.mk
g++ -O3 -march=native -I/usr/share/verilator/include -I obj_dir /usr/share/verilator/include/verilated.cpp verilator_testbench.cpp obj_dir/Vtester__ALL.a  -o tester.out

#-I/home/philippos/Loki/systemc-2.3.1/include/

./tester.out
rm tester.out obj_dir/*
rmdir obj_dir



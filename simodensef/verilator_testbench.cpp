
#include <stdio.h>
#include <stdlib.h>

#include "verilated.h"
//#include <verilated_vcd_c.h>

//#include "verilated_vcd_sc.h"
//#include "systemc.h"

#include "Vtester.h"

// https://stackoverflow.com/questions/23265265/binary-to-float/23266168
float bintofloat(unsigned int x) {
    union {
        unsigned int  x;
        float  f;
    } temp;
    temp.x = x;
    return temp.f;
}

unsigned int floattobin(float f) {
    union {
        unsigned int  x;
        float  f;
    } temp;
    temp.f = f;
    return temp.x;
}

float inttofloat(int x) {
    union {
        int  x;
        float  f;
    } temp;
    temp.x = x;
    return temp.f;
}


int main(int argc, char **argv) {

	Verilated::commandArgs(argc, argv);
	
	
	//Verilated::traceEverOn(true);
    //VerilatedVcdC* tfp = new VerilatedVcdC;
    //topp->trace(tfp, 99);  // Trace 99 levels of hierarchy
    //tfp->open("simx.vcd");
	

	printf("Simulation starting...\n");
	//Vrtl *r = new Vrtl;
	Vtester *r= new Vtester;
	
	//Top_Level()
	
	r->clk = 1;
	r->reset = 1;
	//(r->clk).write(false);	
	
	int k=0;
	
	for(k=0; k<100; k++) {
		r->clk = !(r->clk);			
		r->eval();	
	}
	
	r->reset = 0;
	
	unsigned int shift_register [10] = {0};
	
	while (true) {
		r->clk = !(r->clk);
		
		if (r->clk==0){
			for (int i=9; i>0; i--)
				shift_register[i]=shift_register[i-1];
			r->fpuout=shift_register[5];
				
			if (r->fpuen == 0x1) {
				shift_register[0] = floattobin(bintofloat(r->fpu1in) + bintofloat(r->fpu2in));
				//printf("adding %f to %f\n", bintofloat(r->fpu1in),bintofloat(r->fpu2in));
				//printf("adding %f to %f\n", bintofloat(r->fpu1in),bintofloat(r->fpu2in));
			}
			if (r->fpuen == 0x2)
				shift_register[0]= floattobin(bintofloat(r->fpu1in) * bintofloat(r->fpu2in));
			if (r->fpuen == 0x4) 
				shift_register[0]= floattobin(bintofloat(r->fpu1in) / bintofloat(r->fpu2in));
			if (r->fpuen == 0x8) {
				shift_register[0]= (int) bintofloat(r->fpu1in);
			}
			if (r->fpuen == 0x10) 
				shift_register[0]= floattobin((float) r->fpu1in);
		}
		
			
		r->eval();			
		if (r->finished) break;
		k=(k+1)%16384;
		if (k==0){ fflush(stdout); }
//std::cout << std::flush;

	}
	
	printf("Simulation ended...\n");
	
}

`define DEB 0
`define STDO 1

`include "cpu.v"
`include "caches.v"
`include "custom.v"

module System(clk, reset, StartAddress, StackPointer,      
        addrD, doutDstrobe, doutD, dinDstrobe, dinD, enD, weD, readyD, accR, accW,
        fpu1in, fpu2in, fpuen, fpuout,
        debug, flush, flushed);
	input clk, reset;	
	
	input [`IADDR_bits-1:0] StartAddress;
    input [`DADDR_bits-1:0] StackPointer;
	        
    output [`DADDR_bits-1:0] addrD;
    output [`DL2subblocks_Log2-1:0] doutDstrobe;
    output [`DL2block/`DL2subblocks-1:0] doutD;
    input [`DL2subblocks_Log2-1:0] dinDstrobe;
    input [`DL2block/`DL2subblocks-1:0] dinD;    
    output enD;
    output weD;
    input readyD;
    input accR;
    input accW;
    
    output [32-1:0] fpu1in;
    output [32-1:0] fpu2in;
    output [5-1:0] fpuen;
    input  [31:0] fpuout;
    
    output [31:0] debug;
    input flush;
    output flushed;      
    
    wire [`IADDR_bits-1:0] addrA;
    wire [31:0] doutA;    
    
    wire [`DADDR_bits-1:0] addrB;
    wire [`VLEN-1:0] dinB;
    wire [`VLEN-1:0] doutB;   
     
    wire enB;
    wire [`VLEN/8-1:0] weB;
    wire readyB;
    
    wire acceptingB;
    
    
    //wire [`IADDR_bits-1:0] miaddr; assign miaddr=(reset)?addr:addrA; 
	//wire miwe; assign miwe = (reset)?we[0]:0;
	//wire mien; assign mien = 0;//(reset)?en:0;
	//MemoryI mi (clk, reset, miaddr, mien, miwe, din, doutA);
	wire readyA;
	wire enI; wire [`IADDR_bits-1:0] addrI; wire [`VLEN-1:0] doutI; wire readyI;
	
	/*IL1Cache*/IL1CacheWithImaginaryBL1Cache il1 (clk, reset, addrA, doutA, readyA,
		enI, addrI, doutI, readyI);

    /*wire [`DADDR_bits-1:0] mdaddr; assign mdaddr=(reset)?addr:addrB; 
    wire mden; assign mden=(reset)?en:enB;
    wire [`VLEN/8-1:0] mdwe; assign mdwe = (reset)?0:weB;*/
    
    
    wire [`DADDR_bits-1:0] addrC;
    wire [`VLEN-1:0] dinC;
    wire [`VLEN-1:0] doutC;      
    wire enC;
    wire weC;
    wire dreadyC;    
    wire acceptingC; //assign acceptingC=!(enC||weC);    
        
    wire flush_l2;
    wire [31:0] cycles;
    
    DL1cache dc1(clk, reset, cycles,
		addrB, enB, weB, dinB, doutB, readyB, acceptingB, flush,
		addrC, enC, weC, dinC, doutC, dreadyC, acceptingC, flush_l2);     
    
   
    //wire acceptingD; assign acceptingD=!(enD||weD);
    
    
    DL2cacheU dc2(clk, reset, 
    	enI, addrI, doutI, readyI,
		addrC, enC, weC, dinC, doutC, dreadyC, acceptingC, flush_l2,
		addrD, enD, weD, dinDstrobe, dinD, doutDstrobe, doutD, readyD, accR, accW, flushed); 		
    
    //MemoryD3 md(clk, reset, addrD, enD, weD, dinD, doutD, dreadyD);
    wire halt;    
    Core c0(clk, reset, cycles,    	
		addrA, doutA, readyA, StartAddress, StackPointer,
		addrB, doutB, dinB, enB, weB, readyB, acceptingB, 
		fpu1in, fpu2in, fpuen, fpuout,
		halt
	);    
   
	//assign dout=0;//doutB;
	assign debug={halt,addrA};//{3'b0,acceptingD,3'b0,acceptingC,3'b0,acceptingB
	   //,doutA[7:0],addrA[11:0]}; //PC
endmodule // System8




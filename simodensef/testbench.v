`define StackPointer 32'h0ffffff0
//`define StartAddress 32'h00008000 //
`define StartAddress 32'h000100ec //32'h000103dc
`include "system.v"

`define BMEM_capacity 16*16*16*16*16*16*16
`define ReadLatency 5
`define WriteLatency 10 // must be > `DL2subblocks

module MemoryD3 (clk, reset, addr, en, we, dinDstrobe, din, doutDstrobe, dout, dready, accR, accW);
	input clk, reset;
	input [`DADDR_bits-1:0] addr;	
    input en;
    input we;    
    input  [`DL2subblocks_Log2-1:0] dinDstrobe;
    input [`DL2block/`DL2subblocks-1:0] din;
    output reg  [`DL2subblocks_Log2-1:0] doutDstrobe;
    output reg [`DL2block/`DL2subblocks-1:0] dout;
    output reg dready;
    output wire accR;
    output wire accW;
    
    (* ram_style = "block" *) reg [`DL2block-1:0] block_ram [`BMEM_capacity/(`DL2block/8)-1:0];
    reg [`DL2block-1:0] rdata;


	always @( posedge clk ) begin		
		if (we) begin
			block_ram[addr>>(`DL2block_Log2-3)]
				[`DL2block/`DL2subblocks*(dinDstrobe+1)-1-:`DL2block/`DL2subblocks]<=din;
			//if(i==0) 
			//if (`DEB)if (dinDstrobe==`DL2subblocks-1) 
			//	$display("DRAMstore %h at waddr %h w %h",{din,block_ram[addr>>(`DL2block_Log2-3)][`DL2block-`DL2block/`DL2subblocks-1:0]}, addr);
		end else begin            
			rdata<=block_ram[addr>>(`DL2block_Log2-3)];
			//if (`DEB) if(en) $display("DRAMload %h from raddr %h index",block_ram[addr>>(`DL2block_Log2-3)], (addr>>(`DL2block_Log2-3))<<(`DL2block_Log2-3),addr>>(`DL2block_Log2-3)); 	
			//$display("DRAM %h",block_ram[1]);
		end
		
	end

	reg [`ReadLatency-1:0] lat;
	reg [`WriteLatency-1:0] latw;
	assign accR=(lat==0 && send_strobe==0);
	assign accW=(latw==0);//&!en;

	wire [`DL2block/`DL2subblocks-1:0] subblock [`DL2subblocks-1:0];
	genvar i;
	for (i=0; i<`DL2subblocks; i=i+1) begin
		assign subblock[i]=rdata[(`DL2block/`DL2subblocks)*(i+1)-1-:`DL2block/`DL2subblocks];
	end
	
	reg [`DL2subblocks_Log2-1:0] send_strobe;
	always @( posedge clk ) begin
		if (reset) begin
			dready<=0;
			lat<=0;latw<=0; doutDstrobe<=0; send_strobe<=0;
		end else begin
			if (en) lat<=(lat<<1)|en; else lat<=(lat<<1);
			if (we) latw<=(latw<<1)|we; else latw<=(latw<<1);
			
			dready<=0; doutDstrobe<=0;		
			if(lat[`ReadLatency-1]||(send_strobe!=0)) begin
				dready<=1;
				doutDstrobe<=send_strobe;
				send_strobe<=send_strobe+1;
				dout<= subblock[send_strobe];
				//if (send_strobe==`DL2subblocks-1) dready<=0;
			end 
			
		end
	end
		// synthesis translate_off	
	integer fd, byte_address;
	reg [7:0] value;	
	reg [`DL2block-1:0] word;
	
	initial begin 

		//byte_address=32'h00010000;
		//byte_address=32'h00010074;
		byte_address=32'h00008000;
	    //fd = $fopen("firmware/firmware.bin", "rb");
	    //fd = $fopen("firmware/firmware.elf", "rb");
	    //fd = $fopen("benchmarks/merge/firmware.bin", "rb");
	    //fd = $fopen("benchmarks/chunksort/firmware.bin", "rb");
	    //fd = $fopen("benchmarks/sort/firmware.bin", "rb");	   
	    //fd = $fopen("benchmarks/sort/firmware.elf", "rb"); 
	    //fd = $fopen("benchmarks/prefixsum/firmware.bin", "rb"); 
	    //fd = $fopen("benchmarks/embench-iot/bd/src/primecount/firmware.bin", "rb"); 
	    fd = $fopen(`BIN, "rb");
	    //fd = $fopen("benchmarks/benchmark-dhrystone/dhrystone", "rb");
	    //fd = $fopen("benchmarks/benchmark-dhrystone/dhrystone.bin", "rb");
	    //fd = $fopen("benchmarks/riscv-coremark/riscv32/firmware.bin", "rb");
	    //fd = $fopen("benchmarks/STREAM/stream.bin", "rb");
	    
	    if (!fd) $error("could not read file");
	    while (!$feof(fd)) begin
		     $fread(value,fd);
		     word[(byte_address%(`DL2block/8))*8+8-1-:8]=value;
		     //$display("[%d][%d] %h",byte_address%4,byte_address/4,value);		     	     
		     if (byte_address%(`DL2block/8)==(`DL2block/8)-1) begin
		     	block_ram[byte_address/(`DL2block/8)]=word;
		     	///*if (`DEB)*/$display ("%h at %h %h",word,byte_address/(`DL2block/8),byte_address);
		     	word=0;		     		     	
		     end
		     byte_address=byte_address+1;	
	   end
	   block_ram[byte_address/(`DL2block/8)]=word;
	   //if (`DEB)$display ("%h at %h %h",word,byte_address/(`DL2block/8), byte_address);
	   dout=block_ram[0];
	end
	// synthesis translate_on 
	initial begin		
		//if (`DEB)$dumpvars(0,clk,reset,addr, en, we, dinDstrobe, din, doutDstrobe, dout, dready, accR, accW);
	end
endmodule // MemoryD3


// synthesis translate_off
module Top_Level();
	reg clk, reset;	

    wire [`DADDR_bits-1:0] addrD;
    wire [`DL2subblocks_Log2-1:0] dinDstrobe;
    wire [`DL2block/`DL2subblocks-1:0] dinD;
    wire [`DL2subblocks_Log2-1:0] doutDstrobe;
    wire [`DL2block/`DL2subblocks-1:0] doutD;       
    wire enD;
    wire weD;
    wire dreadyD;
    
    wire accR;// assign accR=act&&!(enD);
    wire accW;// assign accW=act&&!(weD);
    //wire act;
    
    wire [32-1:0] fpu1in;
    wire [32-1:0] fpu2in;
    wire [5-1:0] fpuen;
    wire  [31:0] fpuout;
    
    reg flush; wire flushed; wire [31:0] debug;
    
    System s0(clk, reset, `StartAddress, `StackPointer,
   	 addrD, dinDstrobe, dinD, doutDstrobe, doutD, enD, weD, dreadyD, accR, accW,
   	 fpu1in, fpu2in, fpuen, fpuout,
     debug, flush, flushed); 
    
    MemoryD3 md(clk, reset, addrD, enD, weD, dinDstrobe, dinD, doutDstrobe, doutD, dreadyD, accR, accW);

	reg [`IADDR_bits-1:0] PCprevt;
	reg [`IADDR_bits-1:0] PCprev;

	integer k;
	initial begin	
		//if (`DEB)$dumpfile("gtkwSystem.vcd");
		//if (`DEB)$dumpvars(0, clk, reset, addrD, flush, flushed);

		PCprev=-1;
		k=1;
		reset=1;clk=0;flush=0;
		repeat(100) begin		
			clk=1; #10 clk=0; #10;
		end
		reset=0;
		
		//repeat(800000/*80000/*00*/) begin			
		while (PCprev!=debug[`IADDR_bits-1:0]) begin	
			PCprevt=debug;
			clk=1; #10 clk=0;  #10;			
			if(!debug[`IADDR_bits]) PCprev=PCprevt;
			k=k+1;	
		end
		
		flush=1; $display("\nFlushing remaing dirty blocks to DRAMs\n");
		clk=1; #10 clk=0; #10;
		flush=0;
		
		//repeat((`DL1sets+`DL2sets)*20*4) begin	
		/*repeat(100) begin	
			clk=1; #10 clk=0; #10;
		end	
		while (!flushed) begin
			clk=1; #10 clk=0; #10;
		end*/	
	end
endmodule //Top_Level
// synthesis translate_on

// synthesis translate_off
module Verilator_Top_Level(clk, reset, finished, fpu1in, fpu2in, fpuen, fpuout);
	input clk;
	input reset;
	output reg finished;
	
	output [32-1:0] fpu1in;
    output [32-1:0] fpu2in;
    output [5-1:0] fpuen;
    input  [31:0] fpuout;  

    wire [`DADDR_bits-1:0] addrD;
    wire [`DL2subblocks_Log2-1:0] dinDstrobe;
    wire [`DL2block/`DL2subblocks-1:0] dinD;
    wire [`DL2subblocks_Log2-1:0] doutDstrobe;
    wire [`DL2block/`DL2subblocks-1:0] doutD;       
    wire enD;
    wire weD;
    wire dreadyD;
    
    wire accR;// assign accR=act&&!(enD);
    wire accW;// assign accW=act&&!(weD);
    //wire act; 
    
    reg flush=0; wire flushed; wire [31:0] debug;
    
    System s0(clk, reset, `StartAddress, `StackPointer,
   	 addrD, dinDstrobe, dinD, doutDstrobe, doutD, enD, weD, dreadyD, accR, accW,
   	 fpu1in, fpu2in, fpuen, fpuout,
     debug, flush, flushed); 
    
    MemoryD3 md(clk, reset, addrD, enD, weD, dinDstrobe, dinD, doutDstrobe, doutD, dreadyD, accR, accW);

	reg [`IADDR_bits-1:0] PCprevt;
	reg [`IADDR_bits-1:0] PCprev;

	integer k;
	
	always @( posedge clk ) begin
		if (reset) begin
			finished <=0; PCprev=-1;
		end else begin				
			if(!debug[`IADDR_bits]) 
				PCprev=PCprevt;
			PCprevt=debug;
			finished <= PCprev==debug[`IADDR_bits-1:0];		
		end
	end
	
endmodule //Top_Level
// synthesis translate_on

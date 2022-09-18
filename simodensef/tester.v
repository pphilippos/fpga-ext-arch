`include "./testbench.v"

module tester(clk, reset, finished, fpu1in, fpu2in, fpuen, fpuout);
	input clk, reset;
	output finished;
	
	output [32-1:0] fpu1in;
    output [32-1:0] fpu2in;
    output [5-1:0] fpuen;
    input  [31:0] fpuout;
    
	//reg valid;
	//reg [31:0] data;
	
	//Modules m(clk, valid, data);	
	Verilator_Top_Level t(clk, reset, finished, fpu1in, fpu2in, fpuen, fpuout);

	always @(posedge clk) begin 
		//data<=data+1;
		//valid<=(data+1)%2;			
	end
	initial begin		
		//$display("Started!");
		//$display($time);
		//data=0;
	end
endmodule

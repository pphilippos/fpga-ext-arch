module rmux#(parameter M=4)(input [M-1:0] CFG, input [M**2-1:0] I, output X);
	assign X = I[CFG];
endmodule

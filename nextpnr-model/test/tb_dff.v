module testbench;

reg clk, rst, shift;
reg [`CFG_HEIGHT-1:0] cdata;

reg pcpi_valid;
reg [31:0] pcpi_insn, pcpi_rs1, pcpi_rs2;
wire pcpi_wr, pcpi_wait, pcpi_ready;
wire [31:0] pcpi_rd;

`include "tb_common.vh"

initial begin
	pcpi_valid = 0;
	pcpi_insn = 0;
	pcpi_rs1 = 0;
	pcpi_rs2 = 0;
	load_bitstream();
	pcpi_rs1 = 32'hffff0000;
	pcpi_rs2 = 32'h55555555;
	#5;
	repeat (5) begin
		clk = 1'b1;
		#5;
		clk = 1'b0;
		#5;
	end
	$display("%h", pcpi_rd);
	$finish;
end

fpga dut (
    .clk(clk), .rst(rst), .shift(shift),
    .cdata(cdata),
    .pcpi_valid(pcpi_valid),
    .pcpi_insn(pcpi_insn), .pcpi_rs1(pcpi_rs1), .pcpi_rs2(pcpi_rs2),
    .pcpi_wr(pcpi_wr), .pcpi_wait(pcpi_wait), .pcpi_ready(pcpi_ready),
    .pcpi_rd(pcpi_rd)
);

endmodule
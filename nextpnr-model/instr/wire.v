// Minimal test
module top;
	wire clock, reset;
	wire [31:0] insn, rs1, rs2, rd;
	wire din_valid, dout_valid;
	(* keep *) PCPI_IF pcpi(
		.pcpi_clock(clock),
		.pcpi_reset(reset),
		.pcpi_valid(din_valid),
		.pcpi_insn(insn),
		.pcpi_rs1(rs1),
		.pcpi_rs2(rs2),
		.pcpi_wr(dout_valid),
		.pcpi_rd(rd),
		.pcpi_wait(1'b0),
		.pcpi_ready(dout_valid),
	);

	assign dout_valid = din_valid;
	assign rd = rs1;

endmodule

task load_bitstream;
	reg [`CFG_DEPTH-1:0] bitstream [`CFG_HEIGHT-1:0];
	integer i, j;
	begin
		$display("Loading bitstream...");
		$readmemb("futurefpga.bit", bitstream);
		clk = 1'b0;
		rst = 1'b1;
		#5;
		clk = 1'b1;
		#5;
		clk = 1'b0;
		shift = 1'b1;
		for (i = `CFG_DEPTH - 1; i >= 0; i = i - 1'b1) begin // backwards
			for (j = 0; j < `CFG_HEIGHT; j = j + 1'b1) begin
				cdata[j] = bitstream[j][i];
			end
			#5;
			clk = 1'b1;
			#5;
			clk = 1'b0;
		end
		#5;
		rst = 1'b0;
		shift = 1'b0;
		$display("Done");
	end
endtask

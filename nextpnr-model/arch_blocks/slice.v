module futurefpga_slice(
    input CLK, RST,
    input [3:0] I,
    output O,
    input [19:0] CFG
);
    wire [15:0] INIT = CFG[15:0];
    wire FF_USED     = CFG[16];
    wire FF_ISEL     = CFG[17];
    wire CARRY_EN    = CFG[18]; // TODO: carry
    wire CIN_CONST   = CFG[19];

    // LUT (X-safe)
    wire [7:0] s3 = I[3] ?     INIT[15:8] :     INIT[7:0];
    wire [3:0] s2 = I[2] ?       s3[ 7:4] :       s3[3:0];
    wire [1:0] s1 = I[1] ?       s2[ 3:2] :       s2[1:0];
    wire lut_out  = I[0] ?          s1[1] :         s1[0];

    wire lut_gate = RST ? 1'b0 : lut_out;

    wire ff_in = FF_ISEL ? I[3] : lut_gate;
    reg ff_out = 0;

    always @(posedge CLK)
        ff_out <= ff_in;

    assign O = FF_USED ? ff_out : lut_gate;
endmodule

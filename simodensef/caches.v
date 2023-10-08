// In powers of 2 above 1
`define IL1sets 64

`define DL1sets 32
`define DL1ways 4

`define DL2sets 32
`define DL2ways 4
`define DL2block 16384 // 512-byte
`define DL2subblocks 32

`define IL1setsLog2 $clog2(`IL1sets)
`define DL1setsLog2 $clog2(`DL1sets)
`define DL1waysLog2 $clog2(`DL1ways)

`define DL2setsLog2 $clog2(`DL2sets)
`define DL2waysLog2 $clog2(`DL2ways)
`define DL2block_Log2 $clog2(`DL2block)
`define DL2subblocks_Log2 $clog2(`DL2subblocks)

//`define BL1mode 0 // 0: per ext. instr. 1: per group, 2: per ext.
//`define BL1lat 0
//`define BL1ways (`BL1mode==0?6:(`BL1mode==1?3:1))
`define BL1waysLog2 $clog2(`BL1ways)

module IL1CacheWithImaginaryBL1Cache (clk, reset, PC, instr, ready,
	en, PCB, instrB, readyB);
	input clk, reset;
	input [`IADDR_bits-1:0] PC;  
    output reg [31:0] instr;
    output reg ready;
    
    output reg en;
    output reg [`IADDR_bits-1:0] PCB;
    input [`VLEN-1:0] instrB;
    input readyB;

    /*(* ram_style = "distributed" *)*/ reg [`VLEN-1:0] mem [`IL1sets-1:0];
    reg [`IADDR_bits-`IL1setsLog2-`VLEN_Log2+3-1:0] tag_array [`IL1sets-1:0];
    reg valid [`IL1sets-1:0]; // this could be useless 
    
    wire [`IL1setsLog2-1:0] set; assign set = PC>>(`VLEN_Log2-3);
    wire [`IADDR_bits-`IL1setsLog2-`VLEN_Log2+3-1:0] tag; assign tag = PC>>(`VLEN_Log2-3+`IL1setsLog2);
    
    wire hit; assign hit = valid[set] && (tag_array[set]==tag);
    reg [`VLEN_Log2-5-1:0] roffset; //assign roffset = PC>>2;
    
    reg [5+3+5-1:0] bl1_tag_array [`BL1ways-1:0];
    reg bl1_valid [`BL1ways-1:0];
	reg bl1_hit; 
	
	reg [`BL1ways-1:0] bl1_nru_bit; reg zero_found; reg [`BL1waysLog2-1:0] candidate;
	
	reg [5+3+5-1:0] opcode_plus;
	reg in_M_or_F;
	
	reg [`BL1lat-1:0] bl1_delay; //reg bl1_was_miss;	
	reg [`BL1lat-1:0] bl1_delay_hits; //(normally with BL1lathit, but that can be 0)
	
	reg pending; integer i,j;  
	
	always @( posedge clk ) begin
		if (reset) begin
			for (i=0; i<`IL1sets; i=i+1) begin
				valid[i]<=0;	
				tag_array[i]<=0;
			end
			pending<=0;	en<=0;
			bl1_delay<=0; bl1_nru_bit<=0; //ready<=1;
			bl1_delay_hits<=0; //bl1_was_miss<=0;
			
		end else begin
			// Plseudo LRU mechanism for the tags of the fake bitstream cache (L0)
			opcode_plus = readyB?
				{instrB[31+(roffset)*32-:5],
				 instrB[14+(roffset)*32-:3],
				 instrB[06+(roffset)*32-:5]}:
				 
				{mem[set][31+(PC[`VLEN_Log2-5-1+2:2]*32)-:5],
			     mem[set][14+(PC[`VLEN_Log2-5-1+2:2]*32)-:3],
			     mem[set][06+(PC[`VLEN_Log2-5-1+2:2]*32)-:5]};
			in_M_or_F = (opcode_plus[5-1:0] == 5'hC) || // M
						(opcode_plus[5-1:0] == 5'h10)|| // F fma
						(opcode_plus[5-1:0] == 5'h11)|| // F fma
						(opcode_plus[5-1:0] == 5'h12)|| // F fma
						(opcode_plus[5-1:0] == 5'h13)|| // F fma
						((opcode_plus[5-1:0] == 5'h14) 
						 && !(opcode_plus[12-:5]==5'h1c)  // F rest, not fmv
						 && !(opcode_plus[12-:5]==5'h1e));
			
			if (`BL1mode==1) begin // pre-defined instruction groups
				if ((opcode_plus[5-1:0] == 5'h10)|| // F fma
					(opcode_plus[5-1:0] == 5'h11)|| 
					(opcode_plus[5-1:0] == 5'h12)||
					(opcode_plus[5-1:0] == 5'h13)) begin
					opcode_plus[5-1:0]=5'h10;
				end
				if ((opcode_plus[5-1:0] == 5'hC)) begin // M
					opcode_plus[5]=0;
					if (opcode_plus[7:6]== 2'b01)
						opcode_plus[7:6]=0;
				end
				if ((opcode_plus[5-1:0] == 5'h14)) begin // F rest,..
					case (opcode_plus[12-:5])
						//5'h0: opcode_plus[14-:5]=0;
						5'h1: opcode_plus[12-:5]=0;
						//5'h2: opcode_plus[14-:5]=2;
						//5'h3: opcode_plus[14-:5]=3;
						//5'h4: opcode_plus[14-:5]=4;
						5'h5: opcode_plus[12-:5]=4;
						//5'h14: opcode_plus[14-:5]=5'h14;
						//5'h18: opcode_plus[14-:5]=5'h18;
						5'h1a: opcode_plus[12-:5]=5'h18;
					 endcase
				end	
				if (`BL1mode==2) begin 
					opcode_plus[12-:8]=0;
					if (opcode_plus[5-1:0] >= 5'h10)
						opcode_plus[5-1:0] = 5'h14;
				end
					
			end
			//$display("%h", opcode_plus);     
			// Find if there is an opcode hit, otherwise find eviction candidate 		
			//if (ready||readyB/*(bl1_delay==0)*/) begin
			bl1_hit = (`BL1lat==0) || (!in_M_or_F); zero_found = 0;
			for (j=0; j<`BL1ways; j=j+1) begin
				if ((hit||readyB) && bl1_valid[j] && (bl1_tag_array[j]==opcode_plus)) begin
					bl1_hit=1;
					candidate=j;					
				end		
				if ((hit||readyB) && (bl1_nru_bit[j]==0) &&(!zero_found) && (!bl1_hit)) begin
					candidate=j;
					zero_found=1;
				end				
			end
			//end	
			// Update opcode cache (can impact time but not correctness)			
			if (in_M_or_F && ((hit&&ready)||readyB)) begin
				if (bl1_nru_bit=={`BL1ways{1'b1}})
					bl1_nru_bit<=0;
				bl1_nru_bit[candidate]<=1;
				bl1_tag_array[candidate]<=opcode_plus;
				bl1_valid[candidate]<=1;				
			end
			//bl1_hit = 0;
							
			if (((ready && hit)||readyB) && (!bl1_hit)) begin
				bl1_delay <= (bl1_delay <<1) | 1'b1;
			end else begin
				bl1_delay <= (bl1_delay <<1);
			end
			
			if (((ready && hit)||readyB) && (bl1_hit && in_M_or_F) 
				&& (`BL1lathit!=0)) 
			begin
				bl1_delay_hits <= (bl1_delay_hits <<1) | 1'b1;
			end else begin
				bl1_delay_hits <= (bl1_delay_hits <<1);
			end
				 
				 
			if (readyB) begin
				mem[set]<=instrB;
				if (`DEB)$display("filling set %d with %h ready %d",set,instrB, ready);
				instr<=instrB[(roffset+1)*32-1-:32];     
			end //else begin
			else if (hit &&ready)
				instr<=mem[set][(PC[`VLEN_Log2-5-1+2:2]+1)*32-1-:32];
				//$display("non filling");
			//end
			
			//ready<=((hit||readyB) && bl1_hit && (bl1_delay==0)) || bl1_delay[`BL1lat-1];
			//ready<= (hit && bl1_hit && (bl1_delay==0)) || (readyB && (bl1_hit/*||bl1_delay[`BL1lat-1]*/)); 					
			ready<=(((ready&&hit/*&&(bl1_delay==0)*/)||readyB) && bl1_hit &&((!in_M_or_F) || (`BL1lathit==0)) ) 
			|| (bl1_delay/*==1'b1<<`BL1lat-1*/[`BL1lat-1] /*&& bl1_was_miss*/) 
			|| (/*(`BL1lathit!=0)&&*/(bl1_delay_hits/*==1'b1<<`BL1lat-1*/[`BL1lathit-1]) /*&& !bl1_was_miss*/);
			
			//if(readyB && (bl1_delay!=0))$display("error");
			
			if (`DEB) $display("ihit %d roffset %d %h",hit,roffset,PC);

			en<=0;
			if ((/*ready &&*/ !hit) && (!pending) /*&& (bl1_delay==0)*/) begin 
				en<=1;
				pending<=1;
				roffset <= PC>>2;		
				PCB<=PC;
				if (`DEB)$display("requesting %h",PC);
			end
			
			if (readyB) begin
				pending<=0;				
				valid[set]<=1;
				tag_array[set]<=tag; //PCB>>(`VLEN_Log2-3+`IL1setsLog2);
			end	

		end
	end		
	
	initial begin
		//if (`DEB)$dumpvars(0, clk, reset, 
		//PC, instr, ready, en, PCB, instrB, readyB, hit, bl1_hit,bl1_delay);
	end
endmodule

module IL1Cache (clk, reset, PC, instr, ready,
	en, PCB, instrB, readyB);
	input clk, reset;
	input [`IADDR_bits-1:0] PC;  
    output reg [31:0] instr;
    output reg ready;
    
    output reg en;
    output reg [`IADDR_bits-1:0] PCB;
    input [`VLEN-1:0] instrB;
    input readyB;

    /*(* ram_style = "distributed" *)*/ reg [`VLEN-1:0] mem [`IL1sets-1:0];
    reg [`IADDR_bits-`IL1setsLog2-`VLEN_Log2+3-1:0] tag_array [`IL1sets-1:0];
    reg valid [`IL1sets-1:0];
    
    wire [`IL1setsLog2-1:0] set; assign set = PC>>(`VLEN_Log2-3);
    wire [`IADDR_bits-`IL1setsLog2-`VLEN_Log2+3-1:0] tag; assign tag = PC>>(`VLEN_Log2-3+`IL1setsLog2);
    
    wire hit; assign hit = valid[set] && (tag_array[set]==tag);
    reg [`VLEN_Log2-5-1:0] roffset; //assign roffset = PC>>2;
    

	reg pending; integer i;  
	
	always @( posedge clk ) begin
		if (reset) begin
			for (i=0; i<`IL1sets; i=i+1) begin
				valid[i]<=0;	
				tag_array[i]<=0;
			end
			pending<=0;	en<=0;

		end else begin

			if (readyB) begin
				mem[set]<=instrB;
				if (`DEB)$display("filling set %d with %h ready %d",set,instrB, ready);
				instr<=instrB[(roffset+1)*32-1-:32];     
			end else begin
				instr<=mem[set][(PC[`VLEN_Log2-5-1+2:2]+1)*32-1-:32];
				//$display("non filling");
			end
			
			ready<=hit||readyB;		
			if (`DEB)$display("ihit %d roffset %d %h",hit,roffset,PC);

			en<=0;
			if ((!hit) && (!pending)) begin 
				en<=1;
				pending<=1;
				roffset <= PC>>2;		
				PCB<=PC;
				if (`DEB)$display("requesting %h",PC);
			end
			
			if (readyB) begin
				pending<=0;				
				valid[set]<=1;
				tag_array[set]<=tag;//PCB>>(`VLEN_Log2-3+`IL1setsLog2);
			end	

		end
	end		
	
	initial begin
		//if (`DEB)$dumpvars(0, clk, reset, 
		//PC, instr, ready, en, PCB, instrB, readyB, hit);
	end
endmodule

module IL1CacheBlock (clk, reset, PC, instr, ready,
	en, PCB, instrB, readyB);
	input clk, reset;
	input [`IADDR_bits-1:0] PC;  
    output [31:0] instr;
    output reg ready;
    
    output reg en;
    output reg [`IADDR_bits-1:0] PCB;
    input [`VLEN-1:0] instrB;
    input readyB;
        
    //assign PCB=PC;
    // now works with both distributed and block, distributed turned faster
    (* ram_style = "distributed" *) reg [`VLEN-1:0] block_ram [`IL1sets-1:0];
    reg [`IADDR_bits-`IL1setsLog2-`VLEN_Log2+3-1:0] tag_array [`IL1sets-1:0];
    reg valid [`IL1sets-1:0];
    
    wire [`IL1setsLog2-1:0] set; assign set = PC>>(`VLEN_Log2-3);
    wire [`IADDR_bits-`IL1setsLog2-`VLEN_Log2+3-1:0] tag; assign tag = PC>>(`VLEN_Log2-3+`IL1setsLog2);
    
    wire hit; assign hit = valid[set] && (tag_array[set]==tag);
    reg [`VLEN_Log2-5-1:0] roffset; //assign roffset = PC>>2;
    
    reg [`VLEN-1:0] dout; reg [32-1:0] dout_i; reg [32-1:0] doutB_i; 
    
	//assign instr=readyB?instrB[(roffset+1)*32-1-:32]:dout[(roffset+1)*32-1-:32];
	//assign instr=!hit?instrB[(roffset+1)*32-1-:32]:dout[(roffset+1)*32-1-:32];
	
	
	reg pending; integer i;  
	reg [`IL1setsLog2-1:0] bset; 
	reg tick; reg direction;
	always @( posedge clk ) begin
	
		/*if (reset) begin
			dout<=0;
		end else*/ begin
			if (readyB) begin
				block_ram[set]<=instrB;
				//$display(din," ",addr);
			//end else begin   
				if (`DEB)$display("filling set %d with %h ready %d",set,instrB, ready);
				     
			end else begin   
				dout<=block_ram[set];//[(PC[3:2]+1)*32-1-:32];
				//$display("non filling");
			end
			
			if(reset) begin
				tick<=0;//PCB<=0;
			end else begin
				tick<=!tick;
				//PCB<=PC;
			end
						
			direction<=readyB;
			
			if (readyB)	doutB_i<=instrB[(roffset+1)*32-1-:32];
			
		end
	end  

	assign instr=direction?doutB_i:dout_i;
	
	always @(tick) begin
		//if(readyB)
		dout_i=dout[(PCB[`VLEN_Log2-5-1+2:2]+1)*32-1-:32];		
	end
	
	always @( posedge clk ) begin
		if (reset) begin
			for (i=0; i<`IL1sets; i=i+1) begin
				valid[i]<=0;	
				tag_array[i]<=0;
			end
			pending<=0;	en<=0;

			PCB<=0;
		end else begin
			
			ready<=hit||readyB;
			
			if (`DEB)$display("ihit %d roffset %d %h",hit,roffset,PC);

			en<=0;
			if ((!hit) && (!pending)) begin 
				en<=1;
				pending<=1;
				bset<=set;
				roffset <= PC>>2;				
				
				if (`DEB)$display("requesting %h",PC);
			end
			
			if (readyB) begin
				pending<=0;				
				valid[set]<=1;
				tag_array[set]<=tag;
			end	

			PCB<=PC;
			
		end
	end	
	
	initial begin
		//if (`DEB)$dumpvars(0, clk, reset, 
		//PC, instr, ready, en, PCB, instrB, readyB, hit);
	end
endmodule


module DL1cache (clk, reset,cycles, 
		addr, en, we, din, dout, dready, accepting, flush_in,
		addrB, enB, weB, dinB, doutB, dreadyB, acceptingB, flush_out);
	input clk, reset; input[31:0] cycles;
	
	input [`DADDR_bits-1:0] addr;	
    input en;
    input [`VLEN/8-1:0] we;    
    input [`VLEN-1:0] din;
    output reg [`VLEN-1:0] dout;
    output dready;
    output accepting;
    input flush_in;
    
    output reg [`DADDR_bits-1:0] addrB;	
    output reg enB;
    output reg /*[`VLEN/8-1:0]*/ weB;    
    input [`VLEN-1:0] dinB;
    output reg [`VLEN-1:0] doutB;
    input dreadyB;
    input acceptingB;
    output reg flush_out;
    
    (* ram_style = "block" *) reg [`VLEN-1:0] way [`DL1ways-1:0][`DL1sets-1:0] ;
    reg [`VLEN-1:0] rdata [`DL1ways-1:0];
    reg [`VLEN-1:0] wdata;

	reg [`DADDR_bits-(`VLEN_Log2-3)-`DL1setsLog2-1:0] tag_array [`DL1sets-1:0][`DL1ways-1:0];
    reg [`DL1ways-1:0] dirty [`DL1sets-1:0];
    reg [`DL1ways-1:0] valid [`DL1sets-1:0];
    reg [`DL1ways-1:0] nru_bit [`DL1sets-1:0];

    wire [`DADDR_bits-(`VLEN_Log2-3)-`DL1setsLog2-1:0] tag; 
    assign tag = addr>>(`DL1setsLog2+(`VLEN_Log2-3));
    
    reg [`DADDR_bits-(`VLEN_Log2-3)-`DL1setsLog2-1:0] wtag ;
    reg [`DADDR_bits-(`VLEN_Log2-3)-`DL1setsLog2-1:0] wtag_next; reg wvalid;
        
    wire access; assign access = (en||(we!=0));    
   
    reg waiting; reg waiting_en;
    reg flushing; 
    reg [`DL1setsLog2+`DL1waysLog2+1-1:0] writethrough_block;
    
    wire [`DL1setsLog2-1:0] writethrough_set; assign writethrough_set = writethrough_block[`DL1setsLog2+`DL1waysLog2-1:`DL1waysLog2];
    wire [`DL1waysLog2-1:0] writethrough_way; assign writethrough_way = writethrough_block[`DL1waysLog2-1:0];
    
    wire [`DL1setsLog2-1:0] set; assign set = (flushing && !waiting)?writethrough_set:(addr>>(`VLEN_Log2-3)); // modulo implied
    //reg [`DL1setsLog2-1:0] from_auto_wt_set;
    
    reg [`DADDR_bits-(`VLEN_Log2-3)-`DL1setsLog2-1:0] tag_real; 
    reg [`DL1setsLog2-1:0] set_real; 
    always @(*) begin 
    	if (reset) begin 
    		tag_real=0;set_real=0;
    	end else begin
    		if (access) begin     			
    			tag_real=tag; set_real=set;
    		end
    	end
    end
    
    reg [`DL1ways-1:0] we_local; reg [`DL1setsLog2-1:0] baddr;
    reg [`DL1setsLog2-1:0] bset;
    
    reg [`DADDR_bits-1:0] writeback_addr;
	//wire hitw;  assign hitw = (wtag==tag_real) && (set_real==baddr) && wvalid;
	//wire hitw2;  assign hitw2 = (wtag==writeback_addr[`DADDR_bits-1:`DL1setsLog2+`VLEN_Log2-3]) && (writeback_addr[`DL1setsLog2+`VLEN_Log2-3-1:`VLEN_Log2-3]==baddr) && wvalid;	
	
    reg hit;  
    reg miss; 
    reg [`DL1waysLog2-1:0] candidate; //reg [`DL1waysLog2-1:0] last_candidate;
            
    genvar j; integer j_;
    
    reg zero_found;

    
    for (j=0;j<`DL1ways;j=j+1) begin   
   	
		always @(posedge clk) begin
		
			if (we_local[j]) begin
				way[j][baddr]<=wdata;
				//if(i==0)$display("...storing %h at waddr %h w %h",din,  ((addr>>2)<<2),we);
				if (`DEB)$display("...storing1 %h at set %d addr %h",wdata, baddr,{wtag,baddr});				
			end         
			rdata[j]<=way[j][set];
			if (`DEB)if (en&&hit) $display("...loading %h from set %d way %d addr %h", way[j][set], j, set, addr);
				//if(en && (i==0)) $display("...loading %h%h%h%h from raddr %h",block_ram[3][addr/4],block_ram[2][addr/4],block_ram[1][addr/4],block_ram[0][addr/4], ((addr>>2)<<2)+i); 			
		end
	end
	
	//reg [`VLEN-1:0] rdata_updated;
	//assign rdata_updated=(hitw)?wdata:rdata;
	/*always @(*) begin
		if ((wtag==tag_real) && (set_real==baddr) && wvalid)
			rdata_updated=wdata;
		else
			rdata_updated=rdata[candidate];
	end*/

	
	//wire [`VLEN-1:0] writeback_data;
	//assign wdata_updated=(hitw2)?wdata:rdata;
	
	reg we_pending; reg[`VLEN/8-1:0] we_pending_v; reg [`VLEN-1:0] we_pending_data;
	reg en_pending; reg [`VLEN/8-1:0] en_pending_write; reg [`DADDR_bits-1:0] en_pending_addr;
	reg writeback;

	integer i,k;
	
	//reg from_auto_wt; reg waiting; reg waiting_en;
	assign accepting = /*acceptingB &&*/ !(/*en_pending||*/we_pending||we||en||/*writeback||*/(waiting/*^dreadyB*/)||flushing);	
	// writeback in accepting can possibly be removed if careful when 2nd level (now problem with MAXI peripheral, but works in simulation if removed)
	
	reg [`DL1setsLog2-1:0] last_set; reg we_local_prev;	
	reg [(`VLEN_Log2-3)-2-1:0] roffset;

	reg ready;
	wire load_from_prev_lev=dreadyB && waiting_en;//(en_pending_write==0);
	assign dready=ready||load_from_prev_lev;
	
	reg read_once;
	reg [`DL1waysLog2-1:0] hit_way;
	reg [`DL1waysLog2-1:0] miss_way;
	
	wire [`VLEN-1:0] rdata_updated;
	wire hitw;  assign hitw = (wtag==tag_real) && (set_real==baddr) && wvalid;
	
	assign rdata_updated=(hitw)?wdata:rdata[hit_way];
	reg full_line_write_miss;
	
	always @( posedge clk ) begin
		if (reset) begin
			//for (k=0;k<`DL1ways;k=k+1) begin 
				for (i=0; i<`DL1sets; i=i+1) begin				  
				    dirty[i]<=0;
					valid[i]<=0;
					nru_bit[i]<=0;
				end
				//tag_array[i]<=0;
			//end
			en_pending<=0;
			roffset<=0;	
			writethrough_block<=0; wtag<=0; wvalid<=0; we_pending<=0; waiting_en<=0;
			writeback<=0;	waiting<=0; waiting_en<=0; read_once<=0; baddr<=0;
			
			flush_out<=0; flushing<=0; full_line_write_miss<=0;
		end else begin

			we_local <=0; we_pending<=0; ready<=0;
			weB<=0; enB<=0;
			
			if (flush_in) begin flushing<=1; end
			
			last_set<=set; //we_local_prev<=we_local;
			
			if (en) roffset<=addr[(`VLEN_Log2-3)-1:2];
						
			
			hit=0; miss=access; zero_found=0;//candidate=0;
			
			for (j_=0;j_<`DL1ways;j_=j_+1) begin
				if (access && ((tag_array[set][j_]==tag) && valid[set][j_])) begin
					hit=1;
					candidate=j_;
					miss=0;
				end
				if (access && (nru_bit[set][j_]==0) && (!zero_found) && (!hit)) begin
					candidate=j_;
					zero_found=1;
				end
			end	
			
			if (access) begin
				if (`DEB)$display("DL1 Access hit %d set %d", hit, set);
				if ((nru_bit[set] /*|(1<<candidate)*/)=={`DL1ways{1'b1}})
					nru_bit[set]<=0;
				nru_bit[set][candidate]<=1;//!(we=={(`VLEN/8){1'b1}});
			end
			
			if (hit) begin
				if (`DEB)$display("hit1 set %d tag %h way %h",set, tag, candidate);
				if (en) ready<=1;
				
				if (we!=0) begin 
					//baddr<=bset;
					we_pending<=(last_set!=set) && !hitw;
													
					we_pending_v=we<<(addr[(`VLEN_Log2-3)-1:2]*4);	
					wtag_next=tag;bset=set;
					//waddrh<=addr;
					we_pending_data=din<<(addr[(`VLEN_Log2-3)-1:2]*32);
					
					dirty[set][candidate]<=1;											
					
					
					if (`DEB)$display("writeL1 %h at %h was_dirty %h we %h off %d",din,addr, dirty[set][candidate],we, addr[(`VLEN_Log2-3)-1:2]);
				end	
				hit_way=candidate;
				//if ((nru_bit[set] |(1<<candidate))=={`DL1ways{1'b1}})
				//	nru_bit[set]<=0;
				//nru_bit[set][candidate]<=1;
				//if(writeback)$display("ERROR");
			end
			
			if (we_pending  
				||(hit&&(we!=0)&&((last_set==set)|| hitw))
				) begin
				we_local[hit_way]<=1; wvalid<=1; wtag<=wtag_next; baddr<=bset;
				for (i=0; i<`VLEN/8; i=i+1) 
					wdata[(i+1)*8-1-:8]<= we_pending_v[i]?
						we_pending_data[(i+1)*8-1-:8]:
						 ((hitw)?wdata[(i+1)*8-1-:8]:
						 rdata[hit_way][(i+1)*8-1-:8]
						 );	
				if (`DEB)$display("STORE_PEND finished %d",cycles+1);
				//$display("%h %h %h",rdata,wdata, we_pending_data);			
			end
		
			if (miss) begin
				if (`DEB)$display("miss set %d tag %h way %d",set, tag, candidate);
				en_pending_addr<={tag,set,{(`VLEN_Log2-3){1'b0}}};
				//baddr<=set;
				en_pending_write<=we<<(addr[(`VLEN_Log2-3)-1:2]*4);	
				if (`DEB)if (we!=0) 
					$display("writeL1 %h at %h tag %h way %d", din, addr,tag, candidate);		
					
				we_pending_data=din<<(addr[(`VLEN_Log2-3)-1:2]*32);
								
				if (dirty[set][candidate]&&valid[set][candidate]) begin
					//$display("was dirty");
					writeback<=1;
					read_once<=1;
					//from_auto_wt<=0;					
					addrB<={tag_array[set][candidate],set,{(`VLEN_Log2-3){1'b0}}};		
				end else begin 
					enB<=acceptingB; addrB<={tag,set,{(`VLEN_Log2-3){1'b0}}}; //waddrh<=addr;
					en_pending<=!acceptingB;					
				end
				
				valid[set][candidate]<=1;
				dirty[set][candidate]<=we!=0;
				tag_array[set][candidate]<=tag;
				
				waiting<=1;
				waiting_en<=en;
				full_line_write_miss<=0;
				
				if((we=={(`VLEN/8){1'b1}})) begin
					full_line_write_miss<=1;
					en_pending<=0; enB<=0;//waiting<=0;
					waiting<=dirty[set][candidate]&&valid[set][candidate];
					wdata<= din; 
					//nru_bit[set][candidate]<=0;
					we_local[candidate]<=1; wvalid<=1;
					//if (en_pending) 
					if (`DEB)$display("we %h din %h rof %d",we,din, addr[(`VLEN_Log2-3)-1:2]);
					if (`DEB)$display("STORE_PEND finished %d",cycles+1);
					//wdata<= din; we_local[candidate]<=1; 
					//wvalid<=1;
				end
					
				wtag<=tag; baddr<=set;
				
				miss_way<=candidate;
				//nru_bit[set][candidate]<=1;
			end
					
			if (writeback) begin 	
				if (read_once) 
					doutB<=rdata[miss_way];//_updated;
				read_once<=0;
				//$display("%h %h %h %b %h",rdata,wdata, rdata_updated,acceptingB, tag);
				if (acceptingB)	begin
			
					weB<=1;  
					//if (en_pending) 
					//	addrB<={tag_array[last_set],last_set,{(`VLEN_Log2-3){1'b0}}}; 
					
					//if (`DEB)$display("WritebackL1 %h %h f%d (%d,(%d,%d),%d)",flushing?rdata[writethrough_way]:rdata_updated,addrB,flushing,writethrough_set,set,writethrough_way,candidate);
					if (`DEB)$display("WritebackL1 %h at %h",read_once?rdata[miss_way]:doutB, addrB);
														
					if (flushing && !waiting) begin
						doutB<=rdata[writethrough_way];
						dirty[set][writethrough_way]<=0;
						writethrough_block<=writethrough_block+1;
						if (`DEB)$display("DL1writethrough_block %d set %d way %d addr %h", writethrough_block, writethrough_set,writethrough_way,addrB);
									
					end	else begin
						en_pending<=!full_line_write_miss;
						if (full_line_write_miss) waiting<=0;						
					end
							
					writeback<=0;				
				end					
								
			end else if (flushing && acceptingB && !waiting) begin
			
				addrB<={tag_array[set][writethrough_way],set,{(`VLEN_Log2-3){1'b0}}};
				if (dirty[set][writethrough_way]&&valid[set][writethrough_way]) begin	
					writeback<=1;
				end else begin
					writeback<=0;
					writethrough_block<=writethrough_block+1;
					if (`DEB)$display("writethrough_block %d set %d way %d ", writethrough_block, writethrough_set,writethrough_way);	
				end
				
				
				if (writethrough_block[`DL1setsLog2+`DL1waysLog2+1-1]==1) begin
					flush_out<=1;	
					writethrough_block<=0;				
				end												
			end 
			
			if (flush_out) begin flushing<=0; flush_out<=0; end 
									
			if (en_pending && acceptingB) begin // diff addr in MAXI?
				enB<=1; addrB<=en_pending_addr; //waddrh<=en_pending_addr;
				en_pending<=0;
			end
			
			if (dreadyB) begin
				waiting<=0; waiting_en<=0;
				if (`DEB)$display("Reading %h from DL2 addr %h",dinB,addrB);
				we_local[miss_way]<=1; wvalid<=1; //wtag<=addrB>>(`DL1setsLog2+`VLEN_Log2-3); 
				//baddr<=addrB>>(`VLEN_Log2-3);			
				
				for (i=0; i<`VLEN/8; i=i+1) 
					wdata[(i+1)*8-1-:8]<= en_pending_write[i]?
						we_pending_data[(i+1)*8-1-:8]: dinB[(i+1)*8-1-:8];
				if (`DEB && (en_pending_write!=0))$display("STORE_PEND finished %d",cycles+1);
				//nru_bit[addrB[`VLEN_Log2-3+`DL1setsLog2-1:`VLEN_Log2-3]][miss_way]<=0;
			end
		end		
	end	
	
	always @(*) begin
		if (roffset==0) // also case of vectors, where we want the entire block
			dout=load_from_prev_lev? dinB: rdata_updated;
		else
			dout=load_from_prev_lev? dinB[32*(roffset+1)-1-:32]:
					rdata_updated[32*(roffset+1)-1-:32];	
	end
	
	initial begin
		/*if (`DEB)$dumpvars(0, clk, reset, 
		addr, en, we, din, dout, dready, accepting,
		addrB, enB, weB, dinB, doutB, dreadyB, acceptingB,
		accepting,acceptingB,en_pending,writeback,we_local,we_pending,weB,enB,last_set,hitw,
		dready,miss,hit,we,en,ready,waiting,waiting_en, wdata,rdata_updated,wtag,tag,flushing
		);*/
	end
endmodule // DL1cache


module DL2cache (clk, reset, 
		addr, en, we, din, dout, dready, accepting, flush_in,
		addrB, enB, weB, dinBstrobe, dinB, doutBstrobe, doutB, dreadyB, accR, accW, flush_out);
	input clk, reset;
	
	input [`DADDR_bits-1:0] addr;	
    input en;
    input we;    
    input [`VLEN-1:0] din;
    output reg [`VLEN-1:0] dout;
    output dready;
    output accepting;
    input flush_in;
    
    output reg [`DADDR_bits-1:0] addrB;	
    output reg enB;
    output reg /*[`VLEN/8-1:0]*/ weB;    
    input [`DL2subblocks_Log2-1:0] dinBstrobe;
    input [`DL2block/`DL2subblocks-1:0] dinB;
    output reg [`DL2subblocks_Log2-1:0] doutBstrobe;
    output /*reg*/ [`DL2block/`DL2subblocks-1:0] doutB;
    input dreadyB;
    input accR;
    input accW;
    output reg flush_out;
    
    (* ram_style = "block" *) reg [`DL2block/`DL2subblocks-1:0] way [`DL2ways-1:0][`DL2sets*`DL2subblocks-1:0] ;
    reg [`DL2block/`DL2subblocks-1:0] rdata [`DL2ways-1:0];
    reg [`DL2block/`DL2subblocks-1:0] wdata;

	reg [`DADDR_bits-(`DL2block_Log2-3)-`DL2setsLog2-1:0] tag_array [`DL2sets-1:0][`DL2ways-1:0];
    reg [`DL2ways-1:0] dirty [`DL2sets-1:0];
    reg [`DL2ways-1:0] valid [`DL2sets-1:0];
    reg [`DL2ways-1:0] nru_bit [`DL2sets-1:0];

    wire [`DADDR_bits-(`DL2block_Log2-3)-`DL2setsLog2-1:0] tag; 
    assign tag = addr>>(`DL2setsLog2+(`DL2block_Log2-3));
    
    reg [`DADDR_bits-(`DL2block_Log2-3)-`DL2setsLog2-1:0] wtag;
    reg [`DADDR_bits-(`DL2block_Log2-3)-`DL2setsLog2-1:0] wtag_next; reg wvalid;
    
    wire access; assign access = (en||we);  
   
    reg flushing; reg waiting; reg waiting_en;

    reg [`DL2setsLog2+`DL2waysLog2+1-1:0] writethrough_block;
    wire [`DL2setsLog2-1:0] writethrough_set; assign writethrough_set = writethrough_block[`DL2setsLog2+`DL2waysLog2-1:`DL2waysLog2];
    wire [`DL2waysLog2-1:0] writethrough_way; assign writethrough_way = writethrough_block[`DL2waysLog2-1:0];
        
    wire [`DL2setsLog2-1:0] set; assign set = //(flushing && !waiting)?writethrough_set:
    
    										(addr>>(`DL2block_Log2-3)); // modulo implied
    
    wire [`DL2subblocks_Log2-1:0] access_strobe; 
    reg  [`DL2subblocks_Log2-1:0] read_strobe;
    reg  [`DL2subblocks_Log2-1:0] write_strobe;
    assign access_strobe=addr>>(`DL2block_Log2-`DL2subblocks_Log2-3);
    	
	reg [`DADDR_bits-(`DL2block_Log2-3)-`DL2setsLog2-1:0] tag_real; 
    reg [`DL2setsLog2/*+`DL2subblocks_Log2*/-1:0] set_real; 
    always @(*) begin 
    	if (reset) begin 
    		tag_real=0;set_real=0;
    	end else begin
    		if (access) begin
    			tag_real=tag; set_real=set;//subset;
    		end
    	end
    end
    
    wire [`DL2setsLog2+`DL2subblocks_Log2-1:0] subset; 
	assign subset={	(flushing && !waiting)?writethrough_set:set_real,
		/*!(writeback||flushing)*/access?access_strobe:read_strobe};
		
    reg [`DL2ways-1:0] we_local; reg [`DL2setsLog2-1:0] baddr;
    reg [`DL2setsLog2-1:0] bset;
    reg [`DADDR_bits-1:0] writeback_addr;
    	
    reg hit;  
    reg miss; 
    reg [`DL2waysLog2-1:0] candidate;
            
    genvar j; integer j_;
    
    reg zero_found;        

    for (j=0;j<`DL2ways;j=j+1) begin   
   	
		always @(posedge clk) begin
		
			if (we_local[j]) begin
				way[j][{baddr,write_strobe}]<=wdata;
				//if(i==0)$display("...storing %h at waddr %h w %h",din,  ((addr>>2)<<2),we);
				if (`DEB)$display("...storing2 %h at set %d addr%h way %d",wdata, baddr,{wtag,baddr,write_strobe,{(`DL2block_Log2-`DL2subblocks_Log2-3){1'b0}}},j);
			end         
			rdata[j]<=way[j][subset];
			if (`DEB)if (en&&hit &&j==candidate) $display("...loading %h from set %d (%d) way %d addr %h tag %h ", way[j][subset], set, subset, j, addr, tag);
				//if(en && (i==0)) $display("...loading %h%h%h%h from raddr %h",block_ram[3][addr/4],block_ram[2][addr/4],block_ram[1][addr/4],block_ram[0][addr/4], ((addr>>2)<<2)+i); 			
		end
	end
	
	reg we_pending; reg[`DL2block/`DL2subblocks/`VLEN-1:0] we_pending_v; 
	reg [`VLEN-1:0] we_pending_data;
	
	reg en_pending; reg [`DL2block/`DL2subblocks/`VLEN-1:0] en_pending_write; reg [`DADDR_bits-1:0] en_pending_addr;
	reg writeback; 

	integer i;
	
	assign accepting = /*accR&&accW*/ !(/*en_pending||*/we_pending||we||en||/*(writeback)||*/(waiting/*&&!dreadyB*/)||flushing); // waiting necessary?
	// writeback in accepting can possibly be removed if careful when 2nd level (now problem with MAXI peripheral, but works in simulation if removed)
	
	reg [`DL2setsLog2-1:0] last_set; reg we_local_prev;	
	reg [(`DL2block_Log2-3)-(`DL2subblocks_Log2)-(`VLEN_Log2-3)-1:0] roffset;
	
	reg [`DL2subblocks_Log2-1:0] waiting_en_strobe;
	reg ready;
	wire load_from_prev_lev=dreadyB && waiting_en && (waiting_en_strobe==dinBstrobe);//(en_pending_write==0);
	
	
	assign dready=ready||load_from_prev_lev;
	
	reg read_once;
	reg [`DL2waysLog2-1:0] hit_way;
	reg [`DL2waysLog2-1:0] miss_way;
	
	wire [`DL2block-1:0] rdata_updated;
	wire hitw;  assign hitw = (wtag==tag_real) && (subset=={baddr,write_strobe}) && wvalid;
	assign rdata_updated=(hitw)?wdata:rdata[hit_way];
	reg from_writeback;
	
	reg [`DL2waysLog2-1:0] flush_way;
	assign doutB=rdata[(flushing&&!waiting)?flush_way:miss_way];
	reg hitw_saved;	
	
	always @( posedge clk ) begin
		if (reset) begin
			for (i=0; i<`DL2sets; i=i+1) begin
				dirty[i]<=0;
				valid[i]<=0;
				nru_bit[i]<=0;
				//tag_array[i]<=0;
			end
			en_pending<=0; we_pending<=0;
			roffset<=0;	
			writethrough_block<=0;	wvalid<=0; wtag<=0; baddr<=0;
			writeback<=0; waiting<=0; waiting_en<=0; waiting_en_strobe<=0;
			
			flush_out<=0; flushing<=0; read_once<=0; from_writeback<=0; 
			read_strobe<=0; write_strobe<=0; doutBstrobe<=0;flush_way<=0;

		end else begin

			we_local <=0; we_pending<=0; ready<=0;
			weB<=0; enB<=0; flush_out<=0; 

			//load_from_wreg<=0;
			if (flush_in) begin flushing<=1; end
			
			last_set<=set; 
			
			if (en) roffset<=addr[(`DL2block_Log2-`DL2subblocks_Log2-3)-1:(`VLEN_Log2-3)];
			
			
			hit=0; miss=access; zero_found=0;
			for (j_=0;j_<`DL2ways;j_=j_+1) begin
				if (access && ((tag_array[set][j_]==tag) && valid[set][j_])) begin
					hit=1;
					candidate=j_;
					miss=0;
				end
				if (access && (nru_bit[set][j_]==0) && (!zero_found) && (!hit)) begin
					candidate=j_;
					zero_found=1;
				end
			end	
			
			if (access) begin
				if (`DEB)$display("L2 Access hit %d set %d", hit, set);
				if ((nru_bit[set] /*|(1<<candidate)*/)=={`DL2ways{1'b1}})
					nru_bit[set]<=0;
				nru_bit[set][candidate]<=1;
			end
			
			
			if (hit) begin
				if (`DEB)$display("hit set %d tag %h way %h",set, tag, candidate);
				if (en) ready<=1;
				//read_strobe<=0;
				read_strobe<=access_strobe; // needed for rdata_updated
				if (we) begin 
					//baddr<=set;
					we_pending<=1;//(last_set!=set) && !hitw;//(baddr!=set);			
					we_pending_v=addr[(`DL2block_Log2-`DL2subblocks_Log2-3)-1:(`VLEN_Log2-3)];

					we_pending_data=din;
					wtag_next=tag; bset=set;
					
					dirty[set][candidate]<=1;											
					write_strobe<=access_strobe;
					//read_strobe<=access_strobe;
					read_strobe<=0;
					if (`DEB)$display("L2write %h at %h was_dirty %h",din,addr, dirty[set][candidate]);					
					
					//$display("write %h at %h was_dirty %h",din,addr, dirty[set]);
				end	
				hit_way<=candidate;
				hitw_saved<=hitw;
				
			end
			
			if (we_pending  
				//||(hit&&(we!=0)&&((last_set==set)|| hitw/*(baddr==set)*/))
				) begin
				we_local[hit_way]<=1; wvalid<=1; wtag<=wtag_next; baddr<=bset;
				for (i=0; i<`DL2block/`DL2subblocks/`VLEN; i=i+1) 
					wdata[(i+1)*`VLEN-1-:`VLEN]<= we_pending_v==i?
						we_pending_data:(
						 (hitw_saved)?wdata[(i+1)*`VLEN-1-:`VLEN]:
						 rdata[hit_way][(i+1)*`VLEN-1-:`VLEN]);
				//write_strobe<=read_strobe;
				//read_strobe<=0;	 
				//if (`DEB)$display("L2write2 %h at %h was_dirty %b (last_set==set) %d (baddr==set) %d ",rdata_updated,{bset,candidate}, dirty[set],(last_set==set),(baddr==set));				
			end
			
		
			if (miss) begin
				if (`DEB)$display("miss set %d tag %h way %d",set, tag, candidate);
				en_pending_addr<={tag,set,{(`DL2block_Log2-3){1'b0}}};
				//baddr<=set;
				en_pending_write<=addr[(`DL2block_Log2-`DL2subblocks_Log2-3)-1:(`VLEN_Log2-3)];//we<<(addr[(`VLEN_Log2-3)-1:2]*4);	
				//if (we) 
				//	$display("write %h at %h tag %h way %d", din, addr,tag, candidate);	
					
				we_pending_data=din;//<<(addr[(`VLEN_Log2-3)-1:2]*32);
				
				
				// for writeback
				read_once<=0; 					
				doutBstrobe<=0;
							
				if (dirty[set][candidate]&&valid[set][candidate]) begin
					if (`DEB)$display("was dirty");
					writeback<=1;
					read_strobe<=0;
					
					from_writeback<=1;
					addrB<={tag_array[set][candidate],set,{(`DL2block_Log2-3){1'b0}}};		
				end else begin 
					from_writeback<=0;
					enB<=accR&&accW; addrB<={tag,set,{(`DL2block_Log2-3){1'b0}}}; //waddrh<=addr;
					en_pending<=!(accR&&accW);					
				end
				
				valid[set][candidate]<=1;
				dirty[set][candidate]<=we;
				tag_array[set][candidate]<=tag;
				
				waiting<=1;
				waiting_en<=en; waiting_en_strobe<=access_strobe;
				wtag<=tag; baddr<=set; write_strobe<=0;//access_strobe;
				
				miss_way<=candidate;			
			end
			
			
			if (writeback) begin 	
							
				if (accW || (read_strobe!=0))	begin
					
					weB<=1;  //read_once<=1;
					
					doutBstrobe<=read_strobe; read_strobe<=read_strobe+1;	
					flush_way<=writethrough_way;
								
					if (read_strobe==`DL2subblocks-1) begin
						writeback<=0; //read_strobe<=0;	

						//$display("Writeback %h in DRAM %h f%d",flushing?rdata[writethrough_way]:rdata_updated,addrB,flushing);
														
						if (flushing && !waiting) begin
							//doutB<=rdata[writethrough_way];
							dirty[writethrough_set][writethrough_way]<=0;
							writethrough_block<=writethrough_block+1;
							if (`DEB)$display("writethrough_block %d set %d way %d ", writethrough_block, writethrough_set,writethrough_way);	
						end	else begin
							en_pending<=1;
						end
								
					end				
				end					
								
			end else if (flushing && accW && !waiting) begin
			
				addrB<={tag_array[writethrough_set][writethrough_way],writethrough_set,{(`DL2block_Log2-3){1'b0}}};
				read_strobe<=0;		
				if (dirty[writethrough_set][writethrough_way]&&valid[writethrough_set][writethrough_way]) begin	
					writeback<=1;
				end else begin
					writeback<=0;
					writethrough_block<=writethrough_block+1;
					if (`DEB)$display("writethrough_block %d set %d way %d ", writethrough_block, writethrough_set,writethrough_way);	
				end
				
				
				if (writethrough_block[`DL2setsLog2+`DL2waysLog2+1-1]==1) begin
					flush_out<=1;		
					writethrough_block<=0;				
				end												

			end 
			
			if (flush_out) begin flushing<=0; flush_out<=0; end 
			
			if (en_pending && accR && (accW||from_writeback)) begin // diff addr in MAXI?
				enB<=1; addrB<=en_pending_addr; //waddrh<=en_pending_addr;
				en_pending<=0;
			end
			
			
			if (dreadyB) begin				
				write_strobe<=dinBstrobe;
				if (dinBstrobe==`DL2subblocks-1) begin
					waiting<=0; waiting_en<=0; 
				end
				//$display("Reading %h from DRAM addr %h",dinB,addrB);
				we_local[miss_way]<=1; wvalid<=1; //baddr<=addrB>>(`DL2block_Log2-3); wtag<=addrB>>(`DL2setsLog2+`DL2block_Log2-3);
				
				for (i=0; i<`DL2block/`DL2subblocks/`VLEN; i=i+1)
					wdata[`VLEN*(i+1)-1-:`VLEN]<= ((en_pending_write/*[i]*/==i) &&
					 (!waiting_en) && (dinBstrobe==waiting_en_strobe)
					 )?
						we_pending_data/*[(i+1)*8-1-:8]*/: dinB[`VLEN*(i+1)-1-:`VLEN];
				//nru_bit[addrB[`DL2block_Log2-3+`DL2setsLog2-1:`DL2block_Log2-3]][miss_way]<=0;
			end
		end		
	end
	
	
	always @(*) begin
		for (i=0; i<`DL2block/`DL2subblocks/`VLEN; i=i+1)begin
			if (roffset==i) dout=load_from_prev_lev?
				dinB[`VLEN*(i+1)-1-:`VLEN]:rdata_updated[`VLEN*(i+1)-1-:`VLEN];
		end	
	end
	
	initial begin
		/*if (`DEB)$dumpvars(0, clk, reset, 
		addr, en, we, din, dout, dready, accepting,
		addrB, enB, weB, dinB, doutB, dreadyB, accR,en_pending_write,
		accepting,accW,en_pending,writeback,we_local,we_pending,weB,enB,hitw,hitw_saved,
		dready,miss,hit,we,en,ready,waiting,waiting_en, wdata,flushing,we_pending_data,
		read_strobe,write_strobe,access_strobe,dinBstrobe,doutBstrobe,waiting_en_strobe
		);*/
	end
endmodule // DL2cache

module DL2cacheU (clk, reset, 
		enI, addrI, doutI, dreadyI,
		addrD, enD, weD, doutD, dinD, dreadyD, acceptingD, flush_in,
		addrB, enB, weB, dinBstrobe, dinB, doutBstrobe, doutB, dreadyB, accR, accW,
		flush_out);
	
	input clk, reset;	
	
	input enI;
    input [`IADDR_bits-1:0] addrI;
    output [`VLEN-1:0] doutI;
    output dreadyI;
	
	input [`DADDR_bits-1:0] addrD;	
    input enD;
    input weD;    
    output [`VLEN-1:0] doutD;
    input [`VLEN-1:0] dinD;
    output dreadyD;
    output acceptingD;
 	input flush_in;

	output [`DADDR_bits-1:0] addrB;	
    output enB;
    output weB;   
    input [`DL2subblocks_Log2-1:0] dinBstrobe; 
    input [`DL2block/`DL2subblocks-1:0] dinB;
    output [`DL2subblocks_Log2-1:0] doutBstrobe; 
    output [`DL2block/`DL2subblocks-1:0] doutB;
    input dreadyB;
    input accR;
    input accW;
    output flush_out;
   
	reg [`DADDR_bits-1:0] addr;	
    reg en;
    reg we;    
    reg [`VLEN-1:0] din;
    wire [`VLEN-1:0] dout;
    wire dready;
    wire accepting;
    
    
    
    DL2cache dl2u (clk, reset, 
    	addr, en, we, din, dout, dready, accepting, flush_in,
		addrB, enB, weB, dinBstrobe, dinB, doutBstrobe, doutB, dreadyB, accR, accW, flush_out);
		
	assign doutI = dout;
	assign doutD = dout;
	
	reg pending; reg pendingI;
	assign acceptingD = /*accepting &&*/(!(pending||pendingI||enI/*||en||we*/||enD||weD));

	

	reg IorD;
	assign dreadyI = (IorD?dready:0);
	assign dreadyD = (IorD?0:dready);
	
	
	reg [`DADDR_bits-1:0] saddr;	
    reg sen;
    reg swe;   
	
	always @( posedge clk ) begin
		if (reset) begin 
			pending<=0; pendingI<=0;
			en<=0; we<=0; addr<=0; IorD<=0; din<=0; sen<=0;swe<=0;saddr<=0;
		end else begin
			en<=0; we<=0;
			
			if (pendingI && accepting) begin 
				en<=1;
				pendingI<=0;
				IorD<=1;
			end
			
			if (enI) begin
				if (`DEB)$display("Irequest",accepting,dready);
				addr<=addrI;
				en<=accepting;
				pendingI<=!(accepting);
				we<=0;
				if (accepting) IorD<=1;
			end 
			
			if (enD||weD) begin			
				if ((enI || !accepting) ) begin
					saddr<=addrD;
					sen<=enD;
					swe<=weD;
					pending<=1;
				end else begin
					addr<=addrD;
					en<=enD;
					we<=weD;
					IorD<=0;
				end
			end
			
			if (pending && !enI && accepting && !pendingI) begin 
				IorD<=0;
				addr<=saddr;
				en<=sen;
				we<=swe;
				pending<=0;
			end
			if (weD) din<=dinD;
		end
	end   
	initial begin
		//if (`DEB)$dumpvars(0, clk, reset, en,we,enI, enD, weD, dreadyI, dreadyD,  dready,pending,pendingI);
	end	
endmodule // DL2cacheU


// Sorin Jayaweera
// 909 957 6074
// sojayaweera@g.hmc.edu
// 9/6/2025
`timescale 1ns/1ps
module test_sum_v3 (
		output logic [6:0] seg0, 
		output logic [6:0] seg1,
		output logic seg1good,
		output logic seg0good,
		output logic [7:0] clk
	);
		
	// inputs
	logic [3:0] i0;
	logic [3:0] i1;
	logic reset;
	//outputs to test
	logic sel;
	logic nsel;
	logic [6:0] segout;
	logic [4:0] sum;
	
	assign i0 = {clk[4],clk[5],clk[6],clk[7]};
	assign i1 = {clk[0],clk[1],clk[2],clk[3]};
	top dut(i0,i1,reset,sel,nsel,segout,sum);
	
	assign seg1good = seg1 == (segout && sel) || nsel;
	assign seg0good = seg0 == (segout && nsel) || sel;
	
	
	initial
		begin
				clk = 8'b0;
		end
	
	// generate clock
	always 
		begin
		  clk = clk + 1'b1; #1; 
		  
		  if(clk == 8'b111111) begin
				$display("no cases left");
				$stop;
			end
		end
		
	always_comb
		case(i1)
			4'b0000: seg1 = 7'b0000001; //0
			4'b0001: seg1 = 7'b1001111; //1
			4'b0010: seg1 = 7'b0010010; //2
			4'b0011: seg1 = 7'b0000110; //3
			4'b0100: seg1 = 7'b1001100; //4
			4'b0101: seg1 = 7'b0100100; //5
			4'b0110: seg1 = 7'b0100000; //6
			4'b0111: seg1 = 7'b0001111; //7
			4'b1000: seg1 = 7'b0000000; //8
			4'b1001: seg1 = 7'b0001100; //9
			4'b1010: seg1 = 7'b0001000; //A
			4'b1011: seg1 = 7'b1100000; //B
			4'b1100: seg1 = 7'b0110001; //C
			4'b1101: seg1 = 7'b1000010; //D
			4'b1110: seg1 = 7'b0110000; //E
			4'b1111: seg1 = 7'b0111000; //F

		default
			seg1 = 7'b1111111;//7'b0000000;
		endcase
	
	
	always_comb
		case(i0)
			4'b0000: seg0 = 7'b0000001; //0
			4'b0001: seg0 = 7'b1001111; //1
			4'b0010: seg0 = 7'b0010010; //2
			4'b0011: seg0 = 7'b0000110; //3
			4'b0100: seg0 = 7'b1001100; //4
			4'b0101: seg0 = 7'b0100100; //5
			4'b0110: seg0 = 7'b0100000; //6
			4'b0111: seg0 = 7'b0001111; //7
			4'b1000: seg0 = 7'b0000000; //8
			4'b1001: seg0 = 7'b0001100; //9
			4'b1010: seg0 = 7'b0001000; //A
			4'b1011: seg0 = 7'b1100000; //B
			4'b1100: seg0 = 7'b0110001; //C
			4'b1101: seg0 = 7'b1000010; //D
			4'b1110: seg0 = 7'b0110000; //E
			4'b1111: seg0 = 7'b0111000; //F

		default
			seg0 = 7'b1111111;//7'b0000000;
		endcase

	


endmodule

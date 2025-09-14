
// Sorin Jayaweera
// 909 957 6074
// sojayaweera@g.hmc.edu
// 9/6/2025
`timescale 1ns/1ps
module test_sum_v2 (
		output logic good_sum,
		output logic [7:0] clk
	);
		
	// inputs
	logic [3:0] i0;
	logic [3:0] i1;
	
	//outputs to test
	logic sel;
	logic nsel;
	logic [6:0] segout;
	logic [4:0] sum;
	
	//expected output logic
	logic [4:0] esum;
	
	assign i0 = {clk[4],clk[5],clk[6],clk[7]};
	assign i1 = {clk[0],clk[1],clk[2],clk[3]};
	top dut(i0,i1,reset,sel,nsel,segout,sum);
	
	
	
	assign esum = i1 + i0;
	assign good_sum = i1+i0 == sum;
	
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

	


endmodule

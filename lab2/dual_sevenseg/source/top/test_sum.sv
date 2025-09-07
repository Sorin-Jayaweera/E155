// Sorin Jayaweera
// 909 957 6074
// sojayaweera@g.hmc.edu
// 9/6/2025
`timescale 1ns/1ps
module test_sum ();
	logic reset;
	logic [24:0] clk;
	
	// flag to show test success
	logic flag = 1'b0;
	
	// inputs
	logic [3:0] i0;
	logic [3:0] i1;

	
	//outputs to test
	logic sel;
	logic esel;
	
	logic nsel;
	logic ensel;
	
	logic [6:0] segout;
	logic [6:0] esegout = 7'b0;

	logic [4:0] sum;
	logic [4:0] esum;
	
	top dut(i0,i1,reset,sel,nsel,segout,sum);

	initial
		begin
				clk = 25'b0;
		end
	
		// generate clock
	always 
		begin
		  clk = clk + 1'b1; #1; 
		end

	// on each clock cycle, run a test case
	always_comb
		case(clk)
			8'd0: begin				
					reset = 1'b0;
					i0 = 4'b0000;
					i1 = 4'b0000;
					esum = 5'b00000;
					$display("Pass t0: %d", esum == sum);
				end
			8'd1: begin
					reset = 1'b0;
					i0 = 4'b0001;
					i1 = 4'b0000;
					esum = 5'b00001;
					$display("Pass t1: %d", esum == sum);
				end
			8'd2: begin
					reset = 1'b0;
					i0 = 4'b0001;
					i1 = 4'b0001;
					esum = 5'b00010;
					$display("Pass t2: %d", esum == sum);
				end
			8'd3: begin
					reset = 1'b0;
					i0 = 4'b0001;
					i1 = 5'b0010;
					esum = 5'b00011;
					$display("Pass t3: %d", esum == sum);
				end
			8'd4:begin
					reset = 1'b0;
					i0 = 4'b1001;
					i1 = 5'b0010;
					esum = 5'b01011;
					$display("Pass t4: %d", esum == sum);
				end
			8'd5:begin
					reset = 1'b0;
					i0 = 4'b1001;
					i1 = 5'b1010;
					esum = 5'b10011;
					$display("Pass t5: %d", esum == sum);
				end

		default begin
			$display("no cases left");
			$stop;
			end
			//$stop;
		endcase


endmodule
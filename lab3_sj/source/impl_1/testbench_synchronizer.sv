// Sorin Jayaweera
// 909 957 6074
// sojayaweera@g.hmc.edu 
// 9/13/2025

module testbench_debouncer(	);
	
	logic clk;

	logic unstableval;
	logic stableval;
	
	synchronizer dut(clk,unstableval,stableval);

	initial
		begin
			
			unstableval = 4'b0000;
			#20;
			
			assert(stableval == unstableval);
			
			#3;
			unstableval = 4'b1111;
			
			assert(stableval = 4'b0000);
			#8;
			assert(stableval == unstableval);
			
			
		end


	always begin
		clk = 0; #10; clk = 1; #10;
	end

endmodule

module synchronizer(
	input logic clk,
	input logic [3:0] unstableval,
	output logic [3:0] stableval
	);

	always_ff @(posedge clk) begin
		stableval = unstableval;
	end

endmodule
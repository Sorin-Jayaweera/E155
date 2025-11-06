// Sorin Jayaweera
// 909 957 6074
// sojayaweera@g.hmc.edu 
// 9/13/2025

module testbench_synchronizer();
	logic clk;
	logic [3:0] stableval;
	logic [3:0] unstableval;

	synchronizer dut(clk,unstableval,stableval);

	initial
		begin
			unstableval = 4'b0;
			#20;
			
			assert(stableval == unstableval);
			
			#4;
			unstableval = 4'b1111;
			
			assert(stableval == 4'b0000);
			#8;
			assert(stableval == unstableval);
			
			#20;
			
			$stop;
			
		end


	always begin
		clk = 0; #1; clk = 1; #1;
end

endmodule


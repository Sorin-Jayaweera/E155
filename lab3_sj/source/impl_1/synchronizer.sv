module synchronizer(
	input logic clk,
	input logic [3:0] unstableval,
	output logic [3:0] stableval
	);
	
	always_ff @(posedge clk) begin
		stableval = unstableval;
	end

endmodule
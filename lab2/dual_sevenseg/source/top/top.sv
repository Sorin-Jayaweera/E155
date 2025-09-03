module top(
	input logic [3:0] i0,
	input logic [3:0] i1,
	input logic reset,
	output logic [6:0] seg0,
	output logic [6:0] seg1
	);
	
	HSOSC hf_osc (.CLKHFPU(1'b1), .CLKHFEN(1'b1), .CLKHF(int_osc));
	
	logic sel, nsel;
	logic [6:0] segout = 7'b1111111;
	logic [3:0] iActive = 4'b0000;
	
	logic [24:0] counter = 0;
	
	// Look up table for the 7 segment displays
	segLUT lut(.s(iActive),.seg(segout));
	
	// choosing which set of connections for the resource use
	always_ff@(sel, posedge reset) begin
		
			if(reset == 1) begin
					iActive <= 4'b0000;
					seg0 <= 7'b1111111;
					seg1 <= 7'b1111111;
				end
			else begin
				iActive <= sel ? i0 : i1;
				seg0 <= sel ? segout : seg0;
				seg1 <= sel ? seg1 : segout;
				end
	
	end
	
	always_ff@(posedge int_osc, reset) begin
		if(reset) begin
				counter <= 25'b0;
			end
		else begin
			counter <= counter + 1'b1;
		
			sel <= counter[5];
			nsel <= ~sel;
		end
		
end
	

	
endmodule
	
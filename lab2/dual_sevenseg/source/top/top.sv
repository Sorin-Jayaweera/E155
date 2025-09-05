module top(
	input logic [3:0] i0,
	input logic [3:0] i1,
	input logic reset,
	output logic sel,
	output logic nsel,
	output logic [6:0] segout,
	output logic [4:0] sum
	);
	
	HSOSC hf_osc (.CLKHFPU(1'b1), .CLKHFEN(1'b1), .CLKHF(int_osc));
	
	logic [6:0] seg0 = 7'b1111111;
	logic [6:0] seg1 = 7'b1111111;
	
	logic [3:0] iActive = 4'b0000;
	
	logic [24:0] counter = 0;
	
	// Look up table for the 7 segment displays
	segLUT lut(.s(iActive),.seg(segout));
	
	// add the inputs and display on LEDS
	assign sum = i1+i0;//+i1;//fourbitadder adder(i0, i1, sum);
	
	// choosing which set of connections for the resource use
	// sequential logic
	always_ff@(posedge int_osc) begin
		
			if(reset == 1) begin
					iActive <= 4'b0000;
					seg0 <= 7'b1111111;
					seg1 <= 7'b1111111;
				end
			else begin
				iActive <= sel ? i0 : i1;
				seg0 <= sel ? segout : seg0; // previous value OR the new thing
				seg1 <= sel ? seg1 : segout;
				end
	
	end
	// select as a slow clock logic
	always_ff@(posedge int_osc) begin
		if(reset) begin
				counter <= 25'b0;
			end
		else begin
			counter <= counter + 1'b1;
		end
	end
	

	
	assign sel = counter[23];
	assign nsel = !sel;
endmodule
	
// Sorin Jayaweera
// 909 957 6074
// sojayaweera@g.hmc.edu 
// 9/9/2025

// state machine for debounce on press and release
module debouncer( 
	input logic int_osc, 
	input logic  [31:0] counter,
	input logic  pressed,
	input logic  reset,
	input logic  [3:0] itemp,
	output logic [3:0]  i0,
	output logic [3:0] i1
);

	// state logic
	enum logic [1:0] {accepting, entering, exiting} cycleflag; 
	
	
	logic timepassed;
	logic pressedtimepassed;
	
	logic [31:0] countstart;
	logic [31:0] pressedcountstart;
	
	assign timepassed = (counter - countstart ) > 1000000;//6000000;// 0.042 (42ms) * 12000000 (cycles per second) (HFOSC at half speed)
	assign pressedtimepassed = (counter - pressedcountstart) > 1000000;//6000000;


	always_ff@(posedge int_osc,reset) begin
		if(reset == 1'b1) begin
				i0 = 4'b0000;
				i1 = 4'b0000;
				cycleflag = accepting;
				pressedcountstart = counter;
				countstart = counter;
			end
		// pressed the first time, start the press debounce
		// time conditions unnessisary
		else if (pressed & cycleflag == accepting) begin // rising edge, only triggers once bc flag changes
				i1 = i0; //push numbers
				i0 = itemp;
				
				// state logic
				countstart = counter;
				cycleflag = entering;
			end
			
		// has been pressed but on falling edge when released, start release debounce	
		else if (!pressed & timepassed & cycleflag == entering) begin // falling edge
				pressedcountstart = counter;
				cycleflag = exiting;
			end
		
		// release debounce finished, go back to start
		else if( !pressed & pressedtimepassed & cycleflag == exiting)begin
				cycleflag = accepting;
			end
		end
			

endmodule
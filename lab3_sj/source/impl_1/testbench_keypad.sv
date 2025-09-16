module testbench_keypad()
	logic [31:0] counter; 
	logic [3:0] col;
	logic [3:0] row;
	logic pressed;
	logic [3:0] bin;


	keypadhandler kph(counter,col,row,pressed,bin)

	initial begin
		counter = 32'b0;
		col = 4'b0;
	
	end
		
	
	

endmodule
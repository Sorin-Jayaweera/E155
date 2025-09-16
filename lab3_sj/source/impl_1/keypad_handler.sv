// Sorin Jayaweera
// 909 957 6074
// sojayaweera@g.hmc.edu 
// 9/9/2025
//
// checked off
module keypad_handler(
	input logic [31:0] counter, 
	input logic [3:0] col,
	output logic [3:0] row,
	output logic pressed,
	output logic [3:0] bin
	);
	logic [7:0] rowcol;
	logic [3:0] pressedarr;
	logic [1:0] timepressedarr;

	enum logic [5:0] {zero,one,two,three,four,five,six,seven,eight,nine,a,b,c,d,e,f,none} num;
	
	// if there is not an active column for the current row, then output none.
	assign pressed = |pressedarr; // taking the last 4 values of pressed //|pressedarr;
		
	assign rowcol = {row[3],row[2],row[1],row[0],col[3],col[2],col[1],col[0]};
	// scan between activating each of the rows individually
	always_ff@(posedge counter[12]) 
		case(counter[15:14]) // somewhat slow switching between all pins. Sampling the buttons medium fast
			2'b00: begin row = 4'b0001;  pressedarr[0] = (num != none); end
			2'b01: begin row = 4'b0010;  pressedarr[1] = (num != none); end
			2'b10: begin row = 4'b0100;  pressedarr[2] = (num != none); end
			2'b11: begin row = 4'b1000;  pressedarr[3] = (num != none); end
		endcase
		
	always_ff@(posedge counter[10]) begin // holding on to the last buttons for a fairly long time - 1/3 second
		case(counter[16])
			1'b0: begin timepressedarr[0] = |pressedarr; end

		endcase
		
	end

	always_comb
		case(rowcol)
			8'b00010001: num = one;
			8'b00010010: num = two;
			8'b00010100: num = three;
			8'b00011000: num = a;

			8'b00100001: num = four;
			8'b00100010: num = five;
			8'b00100100: num = six;
			8'b00101000: num = b;

			8'b01000001: num = seven;
			8'b01000010: num = eight;
			8'b01000100: num = nine;
			8'b01001000: num = c;

			8'b10000001: num = e;
			8'b10000010: num = zero;
			8'b10000100: num = f;
			8'b10001000: num = d;
		default
			num = none;
		endcase

	always_comb
		case(num)
			zero: bin = 4'b0000;
			one: bin = 4'b0001;
			two: bin = 4'b0010;
			three: bin = 4'b0011;
			four: bin = 4'b0100;
			five: bin = 4'b0101;
			six: bin = 4'b0110;
			seven: bin = 4'b0111;
			eight: bin = 4'b1000;
			nine: bin = 4'b1001;
			a: bin = 4'b1010;
			b: bin = 4'b1011;
			c: bin = 4'b1100;
			d: bin = 4'b1101;
			e: bin = 4'b1110;
			f: bin = 4'b1111;
		default
				bin = 4'b0000;
		endcase
			

			
		

endmodule
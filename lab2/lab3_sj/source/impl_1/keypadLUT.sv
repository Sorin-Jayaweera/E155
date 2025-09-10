// Sorin Jayaweera
// 909 957 6074
// sojayaweera@g.hmc.edu 
// 9/9/2025

module keypadLUT(input logic [3:0] row,input logic [3:0] col, output logic bin);

	enum logic [3:0]{zero, one,two,three,four,five,six,seven,eight,nine,a,b,c,d,e,f} num;
	always_comb
		case({row,col})
			00010001: num = one;
			00010010: num = two;
			00010100: num = three;
			00011000: num = four;
			00100001: num = five;
			00100010: num = six;
			00100100: num = seven;
			00101000: num = eight;

			01000001: num = nine;
			01000010: num = a;
			01000100: num = b;
			01001000: num = c;

			10000001: num = d;
			10000010: num = e;
			10000100: num = f;
			10001000: num = zero;
		default
			num <= zero;
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
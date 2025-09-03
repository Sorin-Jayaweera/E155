module fourbitadder (input logic [3:0] a, input logic [3:0] b, output logic [4:0] sum);

	logic [3:0] c;
	always_comb begin
		sum[0] <= a[0] ^ b[0];
		c[0] <= a[0] & b[0];
		
		sum[1] <= (a[1] ^ b[1]) ^ c[0];
		c[1] <= (a[1] & b[1]) | (a[1] & c[0]) | (b[1] & c[1]);
		
		
		sum[2] <= (a[2] ^ b[2]) ^ c[1];
		c[2] <= (a[1] & b[1]) | (a[1] & c[0]) | (b[1] & c[1]);
		
		
	end
endmodule

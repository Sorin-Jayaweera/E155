module testbench_keypad();
	
	logic [31:0] counter; 
	wire [3:0] col;
	wire [3:0] row;
	logic pressed;
	logic [3:0] binout;
	logic [3:0] bin;
	logic reset;
	
	keypad_handler dut(counter,col,reset,row,pressed,binout);

	
	enum logic [5:0] {zero,one,two,three,four,five,six,seven,eight,nine,a,b,c,d,e,f,none} num;
	logic [7:0] rowcolctrl;

	pulldown(col[0]);
	pulldown(col[1]);
	pulldown(col[2]);
	pulldown(col[3]);

	tranif1 t0(row[0],col[0],num == one);	
	tranif1 t1(row[0],col[1],num == two);
	tranif1 t2(row[0],col[2],num == three);
	tranif1 t3(row[0],col[3],num == a);
	
	tranif1 t4(row[1],col[0],num == four);	
	tranif1 t5(row[1],col[1],num == five);
	tranif1 t6(row[1],col[2],num == six);
	tranif1 t7(row[1],col[3],num == b);
	
	tranif1 t8(row[2],col[0],num == seven);	
	tranif1 t9(row[2],col[1],num == eight);
	tranif1 t10(row[2],col[2],num == nine);
	tranif1 t11(row[2],col[3],num == c);
	
	tranif1 t12(row[3],col[0],num == e);	
	tranif1 t13(row[3],col[1],num == zero);
	tranif1 t14(row[3],col[2],num == f);
	tranif1 t15(row[3],col[3],num == d);


	initial begin
		counter = 0;
		reset = 1;
		#10
		reset = 0;
		//row = 4'b0;
		//col = 4'b0
		
		wait(counter[14] == 1);
		#10
		num = none;
		assert(bin == 4'b0) else $error("null failed");
		wait(counter[14] == 0);

		num = zero;
		wait(counter[14] == 1);
		assert(pressed == 1'b1) else $error("Press detection fault");
		assert(bin == binout) else $error("");
		wait(counter[14] == 0);
		
		num = one;
		wait(counter[14] == 1);
		assert(pressed == 1'b1) else $error("Press detection fault");
		assert(bin == binout) else $error("");
		wait(counter[14] == 1);
		
		num = two;
		wait(counter[14] == 1);
		assert(pressed == 1'b1) else $error("Press detection fault");
		assert(bin == binout) else $error("");
		wait(counter[14] == 0);
		
		num = three;
		wait(counter[14] == 1);
		assert(pressed == 1'b1) else $error("Press detection fault");
		assert(bin == binout) else $error("");
		wait(counter[14] == 0);
		
		num = four;
		wait(counter[14] == 1);
		assert(pressed == 1'b1) else $error("Press detection fault");
		assert(bin == binout) else $error("");
		
		num = five;	
		wait(counter[14] == 1);
		assert(pressed == 1'b1) else $error("Press detection fault");
		assert(bin == binout) else $error("");
		wait(counter[14] == 0);

		num = six;	
		wait(counter[14] == 1);
		assert(pressed == 1'b1) else $error("Press detection fault");
		assert(bin == binout) else $error("");
		wait(counter[14] == 0);		
		
		num = seven;
		
		wait(counter[14] == 1);
		assert(pressed == 1'b1) else $error("Press detection fault");
		assert(bin == binout) else $error("");
		wait(counter[14] == 0);	
		
		num = eight;
		wait(counter[14] == 1);
		assert(pressed == 1'b1) else $error("Press detection fault");
		assert(bin == binout) else $error("");
		wait(counter[14] == 0);
		
		num = nine;
		wait(counter[14] == 1);
		assert(pressed == 1'b1) else $error("Press detection fault");
		assert(bin == binout) else $error("");
		wait(counter[14] == 0);
		
		num = a;
		assert(pressed == 1'b1) else $error("Press detection fault");
		assert(bin == binout) else $error("");
		
		num = b;
		assert(pressed == 1'b1) else $error("Press detection fault");
		assert(bin == binout) else $error("");
		
		num = c;
		assert(pressed == 1'b1) else $error("Press detection fault");
		assert(bin == binout) else $error("");
		num = d;
		assert(pressed == 1'b1) else $error("Press detection fault");
		assert(bin == binout) else $error("");
		
		num = e;
		assert(pressed == 1'b1) else $error("Press detection fault");
		assert(bin == binout) else $error("");
		
		num = f;
		assert(pressed == 1'b1) else $error("Press detection fault");
		assert(bin == binout) else $error("");
	end
		
		
	// drive the clock
	always begin
		counter = counter + 10'b1000000000; #1; counter = counter+10'b1000000000;
	end
	
	
	always_comb 
		case(num)
				zero: begin bin = 4'b0000; rowcolctrl = 8'b10000010; end
				one: begin bin = 4'b0001; rowcolctrl = 8'b00010001; end
				two: begin bin = 4'b0010; rowcolctrl = 8'b00010010; end
				three: begin bin = 4'b0011;rowcolctrl = 8'b00010100; end
				four: begin bin = 4'b0100; rowcolctrl = 8'b00100001; end
				five: begin bin = 4'b0101; rowcolctrl = 8'b00100010; end
				six: begin bin = 4'b0110; rowcolctrl = 8'b00100100; end
				seven: begin bin = 4'b0111; rowcolctrl = 8'b01000001; end
				eight: begin bin = 4'b1000; rowcolctrl = 8'b01000010; end
				nine: begin bin = 4'b1001; rowcolctrl = 8'b01000100; end
				a:  begin bin = 4'b1010; rowcolctrl = 8'b00011000; end
				b: begin bin = 4'b1011; rowcolctrl = 8'b10000010; end
				c: begin bin = 4'b1100; rowcolctrl = 8'b01001000; end
				d: begin bin = 4'b1101; rowcolctrl = 8'b10001000; end
				e: begin bin = 4'b1110; rowcolctrl = 8'b10000001; end
				f: begin bin = 4'b1111; rowcolctrl = 8'b10000100; end
			default
					bin = 4'b0000;
			endcase
endmodule
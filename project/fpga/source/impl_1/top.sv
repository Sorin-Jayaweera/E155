// 11/19/2025
// Sorin Jayaweera
// Frequency counter with LED bucket display
// Counts pulses in 5 ms window and lights up corresponding frequency bucket LED
// Audio-focused distribution: 3 bass, 3 mid, 3 high

module top(
    input  logic reset,
    input  logic square,
    output logic [9:0] led  // led[8:0] = frequency buckets, led[9] = no signal
);

    // Parameters
    localparam CLK_FREQ = 48_000_000;           // 48 MHz
    localparam WINDOW_MS = 5;                    // 5 ms measurement window
    localparam WINDOW_CYCLES = CLK_FREQ * WINDOW_MS / 1000;  // 240,000 cycles
    
    // Internal signals
    logic int_osc;
    logic [17:0] timer;                          // Count to 240,000 (needs 18 bits)
    logic [13:0] pulse_count;                    // Count pulses (up to 10,000, needs 14 bits)
    logic [13:0] pulse_count_latched;            // Store count at end of window
    
    // Synchronizer for square wave input (prevent metastability)
    logic square_sync1, square_sync2;
    logic square_prev;
    logic square_edge;                           // Rising edge detector
    
    // Instantiate 48 MHz oscillator
    HSOSC #(
        .CLKHF_DIV("0b00")  // 48 MHz
    ) hf_osc (
        .CLKHFPU(1'b1),     // Power up
        .CLKHFEN(1'b1),     // Enable
        .CLKHF(int_osc)     // Clock output
    );
    
    // Synchronize input square wave to internal clock (2-stage synchronizer)
    always_ff @(posedge int_osc or posedge reset) begin
        if (reset) begin
            square_sync1 <= 0;
            square_sync2 <= 0;
        end else begin
            square_sync1 <= square;
            square_sync2 <= square_sync1;
        end
    end
    
    // Detect rising edge of synchronized square wave
    always_ff @(posedge int_osc or posedge reset) begin
        if (reset) begin
            square_prev <= 0;
        end else begin
            square_prev <= square_sync2;
        end
    end
    
    assign square_edge = square_sync2 & ~square_prev;
    
    // Timer for 5 ms window
    always_ff @(posedge int_osc or posedge reset) begin
        if (reset) begin
            timer <= 0;
        end else begin
            if (timer >= WINDOW_CYCLES - 1) begin
                timer <= 0;
            end else begin
                timer <= timer + 1;
            end
        end
    end
    
    // Pulse counter
    always_ff @(posedge int_osc or posedge reset) begin
        if (reset) begin
            pulse_count <= 0;
        end else begin
            if (timer == 0) begin
                pulse_count <= 0;  // Reset at start of new window
            end else if (square_edge) begin
                pulse_count <= pulse_count + 1;
            end
        end
    end
    
    // Latch pulse count at end of window (one cycle before timer resets)
    always_ff @(posedge int_osc or posedge reset) begin
        if (reset) begin
            pulse_count_latched <= 0;
        end else begin
            if (timer == WINDOW_CYCLES - 1) begin
                pulse_count_latched <= pulse_count;
            end
        end
    end
	// Decode pulse count to LED bucket (only one LED lights up)
	// Industry-standard audio frequency distribution
	always_ff @(posedge int_osc or posedge reset) begin
		if (reset) begin
			led <= 10'b10_0000_0000;  // No signal LED on at reset
		end else begin
			if (timer == 0) begin  // Update LEDs at start of new window
				if (pulse_count_latched == 0) begin
					led <= 10'b10_0000_0000;  // led[9] = no signal
					
				// BASS RANGE (20-250 Hz) - Foundation & Power
				end else if (pulse_count_latched < 300) begin
					led <= 10'b00_0000_0001;  // led[0] = 20-60 Hz (sub-bass)
				end else if (pulse_count_latched < 625) begin
					led <= 10'b00_0000_0010;  // led[1] = 60-125 Hz (bass fundamentals)
				end else if (pulse_count_latched < 1250) begin
					led <= 10'b00_0000_0100;  // led[2] = 125-250 Hz (upper bass)
					
				// MID RANGE (250-2000 Hz) - Vocals & Instrument Body
				end else if (pulse_count_latched < 2500) begin
					led <= 10'b00_0000_1000;  // led[3] = 250-500 Hz (low-mid warmth)
				end else if (pulse_count_latched < 5000) begin
					led <= 10'b00_0001_0000;  // led[4] = 500-1000 Hz (mid clarity)
				end else if (pulse_count_latched < 7500) begin
					led <= 10'b00_0010_0000;  // led[5] = 1000-1500 Hz (upper-mid presence)
				
				// HIGH RANGE (1500-2000 Hz) - Definition & Attack
				// Split last 500 Hz into 3 bins for high-frequency detail
				end else if (pulse_count_latched < 8333) begin
					led <= 10'b00_0100_0000;  // led[6] = 1500-1667 Hz
				end else if (pulse_count_latched < 9167) begin
					led <= 10'b00_1000_0000;  // led[7] = 1667-1833 Hz
				end else begin
					led <= 10'b01_0000_0000;  // led[8] = 1833-2000 Hz
				end
			end
		end
	end
endmodule


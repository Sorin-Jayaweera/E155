
// Main module - Spectrum analyzer with smooth persistence/fading
module top(
    input  logic reset_in,    // Active LOW (pressed = 0)
    input  logic square,
    output logic [11:0] led
);
    logic int_osc;
    logic reset;
    
    assign reset = ~reset_in;
    
    // 100ms window at 48 MHz
    localparam [25:0] WINDOW_CYCLES = 26'd4_800_000;
    
    // PWM and fade timing
    localparam [11:0] PWM_PERIOD = 12'd2400;       // 20kHz PWM (48MHz / 2400)
    localparam [22:0] FADE_PERIOD = 23'd375_000;   // ~7.8ms per fade step (2 seconds / 256 steps)
    
    // State machine
    typedef enum logic [1:0] {
        COLLECTING,
        LATCHING
    } state_t;
    
    state_t state;
    
    logic [25:0] timer;
    logic [7:0] edge_count;
    logic [7:0] edge_count_latched;
    logic [3:0] bucket;
    logic [3:0] bucket_display;
    
    logic square_sync, square_prev, square_edge;
    
    // Brightness array - one 8-bit brightness value per bucket (0-255)
    logic [7:0] brightness [11:0];
    
    // PWM counter for brightness control
    logic [7:0] pwm_counter;
    logic [11:0] pwm_timer;
    
    // Fade timer
    logic [22:0] fade_timer;
    
    HSOSC #(.CLKHF_DIV("0b00")) hf_osc (
        .CLKHFPU(1'b1),
        .CLKHFEN(1'b1),
        .CLKHF(int_osc)
    );
    
    // Synchronize square wave input
    synchronizer square_synchronizer (
        .clk(int_osc),
        .reset(reset),
        .async_in(square),
        .sync_out(square_sync)
    );
    
    // Edge detection on square wave
    always_ff @(posedge int_osc, posedge reset) begin
        if (reset) begin
            square_prev <= 1'b0;
        end else begin
            square_prev <= square_sync;
        end
    end
    
    assign square_edge = ~square_sync & square_prev;  // FALLING edge
    
    //===========================================
    // STATE MACHINE
    //===========================================
    always_ff @(posedge int_osc, posedge reset) begin
        if (reset) begin
            state <= COLLECTING;
        end else begin
            case (state)
                COLLECTING: begin
                    if (timer >= WINDOW_CYCLES - 1) begin
                        state <= LATCHING;
                    end
                end
                
                LATCHING: begin
                    state <= COLLECTING;  // Auto-restart
                end
                
                default: state <= COLLECTING;
            endcase
        end
    end
    
    //===========================================
    // DATAPATH: Timer
    //===========================================
    always_ff @(posedge int_osc, posedge reset) begin
        if (reset) begin
            timer <= 26'd0;
        end else begin
            case (state)
                COLLECTING: begin
                    timer <= timer + 26'd1;
                end
                
                LATCHING: begin
                    timer <= 26'd0;  // Reset for next window
                end
            endcase
        end
    end
    
    //===========================================
    // DATAPATH: Edge Counter
    //===========================================
    always_ff @(posedge int_osc, posedge reset) begin
        if (reset) begin
            edge_count <= 8'd0;
        end else begin
            case (state)
                COLLECTING: begin
                    if (square_edge) begin
                        edge_count <= edge_count + 8'd1;
                    end
                end
                
                LATCHING: begin
                    edge_count <= 8'd0;  // Reset for next window
                end
            endcase
        end
    end
    
    //===========================================
    // DATAPATH: Latch Count
    //===========================================
    always_ff @(posedge int_osc, posedge reset) begin
        if (reset) begin
            edge_count_latched <= 8'd0;
        end else if (state == LATCHING) begin
            edge_count_latched <= edge_count;
        end
    end
    
    //===========================================
    // BUCKET DETERMINATION (Combinational)
    //===========================================
    always_comb begin
        bucket = 4'd0;
        
        if (edge_count_latched > 8'd184)       bucket = 4'd11;
        else if (edge_count_latched > 8'd167)  bucket = 4'd10;
        else if (edge_count_latched > 8'd150)  bucket = 4'd9;
        else if (edge_count_latched > 8'd134)  bucket = 4'd8;
        else if (edge_count_latched > 8'd117)  bucket = 4'd7;
        else if (edge_count_latched > 8'd100)  bucket = 4'd6;
        else if (edge_count_latched > 8'd83)   bucket = 4'd5;
        else if (edge_count_latched > 8'd67)   bucket = 4'd4;
        else if (edge_count_latched > 8'd50)   bucket = 4'd3;
        else if (edge_count_latched > 8'd33)   bucket = 4'd2;
        else if (edge_count_latched > 8'd16)   bucket = 4'd1;
        else                                   bucket = 4'd0;
    end
    
    //===========================================
    // LATCH BUCKET FOR DISPLAY
    //===========================================
    always_ff @(posedge int_osc, posedge reset) begin
        if (reset) begin
            bucket_display <= 4'd0;
        end else if (state == LATCHING) begin
            bucket_display <= bucket;
        end
    end
    
    //===========================================
    // BRIGHTNESS MANAGEMENT + FADE TIMER (Combined)
    // Fade over 2 seconds: 256 steps × 7.8ms = 2 seconds
    //===========================================
    always_ff @(posedge int_osc, posedge reset) begin
        if (reset) begin
            fade_timer <= 23'd0;
            for (int i = 0; i < 12; i++) begin
                brightness[i] <= 8'd0;
            end
        end else begin
            // Update fade timer
            if (fade_timer >= FADE_PERIOD - 1) begin
                fade_timer <= 23'd0;
            end else begin
                fade_timer <= fade_timer + 23'd1;
            end
            
            // Priority 1: Set active bucket to full brightness (happens in LATCHING state)
            if (state == LATCHING) begin
                brightness[bucket_display] <= 8'd255;
            end
            
            // Priority 2: Fade all buckets (happens every FADE_PERIOD)
            if (fade_timer >= FADE_PERIOD - 1) begin
                for (int i = 0; i < 12; i++) begin
                    // Don't fade the bucket we just set to full brightness
                    if (!(state == LATCHING && i == bucket_display)) begin
                        if (brightness[i] > 8'd0) begin
                            brightness[i] <= brightness[i] - 8'd1;
                        end
                    end
                end
            end
        end
    end
    
    //===========================================
    // PWM COUNTER - Runs at 20kHz (imperceptible flicker)
    //===========================================
    always_ff @(posedge int_osc, posedge reset) begin
        if (reset) begin
            pwm_timer <= 12'd0;
            pwm_counter <= 8'd0;
        end else begin
            if (pwm_timer >= PWM_PERIOD - 1) begin
                pwm_timer <= 12'd0;
                pwm_counter <= pwm_counter + 8'd1;  // Wraps around at 256
            end else begin
                pwm_timer <= pwm_timer + 12'd1;
            end
        end
    end
    
    //===========================================
    // LED DISPLAY: PWM output based on brightness
    // LED[0] = bucket 11, LED[11] = bucket 0
    // Registered output for cleaner PWM
    //===========================================
    always_ff @(posedge int_osc, posedge reset) begin
        if (reset) begin
            led <= 12'b0;
        end else begin
            for (int i = 0; i < 12; i++) begin
                // LED is on when brightness > pwm_counter
                led[11-i] <= (brightness[i] > pwm_counter);
            end
        end
    end
    
endmodule
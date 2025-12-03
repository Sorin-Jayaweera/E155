module led_dimmer (
    input  logic clk,
    input  logic reset,
    input  logic update_strobe,   // Pulse high to update active bucket
    input  logic [3:0] active_bucket,
    output logic [11:0] led_out
);
    // Constants
    localparam [11:0] PWM_PERIOD  = 12'd2400;     // 20kHz
    localparam [22:0] FADE_PERIOD = 23'd375_000;  // Fade speed

    logic [7:0] brightness [11:0];
    logic [22:0] fade_timer;
    logic [11:0] pwm_timer;
    logic [7:0] pwm_counter;

    // Fading and Brightness Update Logic
    always_ff @(posedge clk, posedge reset) begin
        if (reset) begin
            fade_timer <= 23'd0;
            for (int i=0; i<12; i++) brightness[i] <= 8'd0;
        end else begin
            // 1. Fade Timer Tick
            if (fade_timer >= FADE_PERIOD - 1) fade_timer <= 23'd0;
            else                               fade_timer <= fade_timer + 23'd1;

            // 2. Set new Active Bucket (Priority High)
            if (update_strobe) begin
                brightness[active_bucket] <= 8'd255;
            end

            // 3. Fade Processing (Priority Low)
            if (fade_timer >= FADE_PERIOD - 1) begin
                for (int i=0; i<12; i++) begin
                    // Fade if it's not the one we just turned on
                    if (!(update_strobe && i == active_bucket)) begin
                        if (brightness[i] > 8'd0) 
                            brightness[i] <= brightness[i] - 8'd1;
                    end
                end
            end
        end
    end

    // PWM Generation
    always_ff @(posedge clk, posedge reset) begin
        if (reset) begin
            pwm_timer   <= 12'd0;
            pwm_counter <= 8'd0;
            led_out     <= 12'd0;
        end else begin
            // Timer increment
            if (pwm_timer >= PWM_PERIOD - 1) begin
                pwm_timer   <= 12'd0;
                pwm_counter <= pwm_counter + 8'd1;
            end else begin
                pwm_timer   <= pwm_timer + 12'd1;
            end

            // Output Comparator
            for (int i=0; i<12; i++) begin
                // Flip index (11-i) here if you want logical 0 to be physical LED 11
                led_out[11-i] <= (brightness[i] > pwm_counter);
            end
        end
    end
endmodule
module led_decoder (
    input  logic [7:0] edge_count_latched,
    output logic [3:0] bucket
);
    always_comb begin
        bucket = 4'd0;
        if      (edge_count_latched > 8'd184) bucket = 4'd11;
        else if (edge_count_latched > 8'd167) bucket = 4'd10;
        else if (edge_count_latched > 8'd150) bucket = 4'd9;
        else if (edge_count_latched > 8'd134) bucket = 4'd8;
        else if (edge_count_latched > 8'd117) bucket = 4'd7;
        else if (edge_count_latched > 8'd100) bucket = 4'd6;
        else if (edge_count_latched > 8'd83)  bucket = 4'd5;
        else if (edge_count_latched > 8'd67)  bucket = 4'd4;
        else if (edge_count_latched > 8'd50)  bucket = 4'd3;
        else if (edge_count_latched > 8'd33)  bucket = 4'd2;
        else if (edge_count_latched > 8'd16)  bucket = 4'd1;
        else                                  bucket = 4'd0;
    end
endmodule
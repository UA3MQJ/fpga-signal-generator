module signal_generator(clk50M, signal_out, hs_out, key0, key1, RS232in);

input wire clk50M;
output wire [7:0] signal_out;
output wire hs_out;
input wire key0, key1;
input wire RS232in;

wire clk200M;
osc osc_200M(.inclk0(clk50M), .c0(clk200M));

//reset signal generator
wire rst;
powerup_reset rst_gen(.clk(clk200M), .key(~key0), .rst(rst));

reg [31:0] accumulator;
initial accumulator <= 32'd0;

//rs232 rcvr
wire [31:0] adder_value;
wire [7:0] waveform;
ctrl ctrl_0(.clk(clk200M), .rst(rst), .rx(RS232in), .wf(waveform), .adder(adder_value));

// wave_forms
parameter SINE      = 8'd0;
parameter SAW       = 8'd1;
parameter RAMP      = 8'd2;
parameter TRIA      = 8'd3;
parameter SQUARE    = 8'd4;
parameter SAWTRI    = 8'd5;
parameter NOISE     = 8'd6;

wire [7:0] saw_out = accumulator[31:31-7];
wire [7:0] noise_out = 8'd127; //!
wire [7:0] ramp_out = -saw_out;	
wire [7:0] square_out = (saw_out > 127) ? 8'b11111111 : 1'b00000000;
wire [7:0] saw_tri_out = (saw_out > 7'd127) ?  -saw_out : 8'd127 + saw_out;	
wire [7:0] tri_out = (saw_out>8'd191) ? 7'd127 + ((saw_out << 1) - 9'd511) : 
                     (saw_out>8'd063) ? 8'd255 - ((saw_out << 1) - 7'd127) : 7'd127 + (saw_out << 1);
//sine rom
wire [7:0] sine_out;
sine_rom sine1(.clock(clk200M), .address(saw_out), .q(sine_out));

wire [7:0] signal = (waveform ==     SINE) ? sine_out :
                    (waveform ==      SAW) ? saw_out : 
                    (waveform ==     RAMP) ? ramp_out : 
                    (waveform ==     TRIA) ? tri_out :
                    (waveform ==   SQUARE) ? square_out :
                    (waveform ==   SAWTRI) ? saw_tri_out :
                    (waveform ==    NOISE) ? noise_out : 8'd127; //TODO
						  
assign signal_out = (key1) ? signal : 8'd127; //mute
						  
assign hs_out  =  square_out[0];

always @(posedge clk200M) begin
	accumulator <= accumulator + adder_value;
end

endmodule

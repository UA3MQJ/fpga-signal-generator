module ctrl(clk, rst, rx, wf, adder);
input wire clk, rst, rx;
output wire [7:0] wf; //wave form
output wire [31:0] adder;// adder value

reg [7:0] wf_reg;
initial wf_reg <= 8'd0;

reg [31:0] adder_reg;
initial adder_reg <= 32'd1073741;

assign adder = adder_reg;
assign wf = wf_reg;

//UART
parameter global_clock_freq = 200000000;
parameter baud_rate = 115200;
//бодген - модуль для генерации клока UART
// first register:
// 		baud_freq = 16*baud_rate / gcd(global_clock_freq, 16*baud_rate)
//Greatest Common Divisor - наибольший общий делитель. http://www.alcula.com/calculators/math/gcd/
// second register:
// 		baud_limit = (global_clock_freq / gcd(global_clock_freq, 16*baud_rate)) - baud_freq 


//можно добавить другие значения скоростей
parameter GCD = (baud_rate==115200) ? 12800 : 0;

parameter baud_freq  = 16*baud_rate / GCD;
parameter baud_limit = (global_clock_freq / GCD) - baud_freq;

wire uart_clk;
baud_gen BG(.clock(clk), .reset(rst), .ce_16(uart_clk), .baud_freq(baud_freq), .baud_limit(baud_limit));

//RCV
wire [7:0] uart_command;
wire uart_data_ready;

uart_rx URX(.clock(clk), 
		      .reset(rst),
				.ce_16(uart_clk), 
				.ser_in(rx), 
				.rx_data(uart_command), 
				.new_rx_data(uart_data_ready) );
				
//rcv state machine
parameter SM_READY     = 4'd0;
parameter SM_FRQ_WF    = 4'd1;
parameter SM_FRQ_DDS1  = 4'd2;
parameter SM_FRQ_DDS2  = 4'd3;
parameter SM_FRQ_DDS3  = 4'd4;
parameter SM_FRQ_DDS4  = 4'd5;

//messages
parameter CMD_SETFREQ = 8'd1;

reg [3:0] rcv_state;
initial rcv_state <= SM_READY;

always @ (posedge clk) begin 
	
	if (uart_data_ready==1) begin
		if (rcv_state==SM_READY) begin
			rcv_state = (uart_command==CMD_SETFREQ) ? SM_FRQ_WF : rcv_state;
		end else if (rcv_state==SM_FRQ_WF) begin
			wf_reg <= uart_command;
			rcv_state <= rcv_state + 1'b1;
		end else if (rcv_state==SM_FRQ_DDS1) begin
			adder_reg <= (adder_reg << 8) + uart_command;
			rcv_state <= rcv_state + 1'b1;
		end else if (rcv_state==SM_FRQ_DDS2) begin
			adder_reg <= (adder_reg << 8) + uart_command;
			rcv_state <= rcv_state + 1'b1;
		end else if (rcv_state==SM_FRQ_DDS3) begin
			adder_reg <= (adder_reg << 8) + uart_command;
			rcv_state <= rcv_state + 1'b1;
		end else if (rcv_state==SM_FRQ_DDS4) begin
			adder_reg <= (adder_reg << 8) + uart_command;
			rcv_state <= SM_READY;
		end else begin
			rcv_state <= SM_READY;
		end
	end //ucom_ready
	
end

endmodule

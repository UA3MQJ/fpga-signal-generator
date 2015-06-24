derive_clock_uncertainty
create_clock -period 50MHz -name {clk50M} [get_ports {clk50M}]
create_generated_clock -name {clk1M} -divide_by 50 -source [get_ports {clk50M}] [get_registers {FGD|clk1M}]
create_generated_clock -name {clk25M} -divide_by 2 -source [get_ports {clk50M}] [get_registers {FGD|clk25M}]
create_generated_clock -name {clk1hz} -divide_by 50000 -source [get_ports {clk50M}] [get_registers {FGD|clk1hz}]
set_clock_groups -exclusive -group {clk50M}
set_clock_groups -exclusive -group {clk1M}
set_clock_groups -exclusive -group {clk25M}
set_clock_groups -exclusive -group {clk1hz}

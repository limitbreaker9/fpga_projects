# ----------------------------------------------------------------------------
# ZedBoard On-Board OLED Display Constraints (Bank 13 - 3.3V)
# Modified with _0 suffix to match default Vivado external port names
# ----------------------------------------------------------------------------

# Data/Command Select
set_property PACKAGE_PIN U10  [get_ports oled_dc_0]
set_property IOSTANDARD LVCMOS33 [get_ports oled_dc_0]

# Power Reset
set_property PACKAGE_PIN U9   [get_ports oled_res_0]
set_property IOSTANDARD LVCMOS33 [get_ports oled_res_0]

# Serial Clock
set_property PACKAGE_PIN AB12 [get_ports oled_sclk_0]
set_property IOSTANDARD LVCMOS33 [get_ports oled_sclk_0]

# Serial Data In (MOSI)
set_property PACKAGE_PIN AA12 [get_ports oled_sdin_0]
set_property IOSTANDARD LVCMOS33 [get_ports oled_sdin_0]

# VBAT Enable (Core Power)
set_property PACKAGE_PIN U11  [get_ports oled_vbat_0]
set_property IOSTANDARD LVCMOS33 [get_ports oled_vbat_0]

# VDD Enable (Digital Power)
set_property PACKAGE_PIN U12  [get_ports oled_vdd_0]
set_property IOSTANDARD LVCMOS33 [get_ports oled_vdd_0]
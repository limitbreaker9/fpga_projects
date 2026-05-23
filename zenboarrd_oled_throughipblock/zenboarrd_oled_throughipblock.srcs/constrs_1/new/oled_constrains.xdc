# Onboard ZedBoard OLED Pin Layout Constraints (Custom Port Name: oled)
set_property PACKAGE_PIN AA12 [get_ports oled_pin2_io]; # SDIN (MOSI)
set_property IOSTANDARD LVCMOS33 [get_ports oled_pin2_io]

set_property PACKAGE_PIN AB12 [get_ports oled_pin4_io]; # SCLK
set_property IOSTANDARD LVCMOS33 [get_ports oled_pin4_io]

set_property PACKAGE_PIN U10  [get_ports oled_pin7_io]; # DC
set_property IOSTANDARD LVCMOS33 [get_ports oled_pin7_io]

set_property PACKAGE_PIN U9   [get_ports oled_pin8_io]; # RES
set_property IOSTANDARD LVCMOS33 [get_ports oled_pin8_io]

set_property PACKAGE_PIN U11  [get_ports oled_pin9_io]; # VBAT
set_property IOSTANDARD LVCMOS33 [get_ports oled_pin9_io]

set_property PACKAGE_PIN U12  [get_ports oled_pin10_io]; # VDD
set_property IOSTANDARD LVCMOS33 [get_ports oled_pin10_io]

# Tie off the unused Pins 1 & 3 to dummy test points on the JA Pmod header so Vivado compiles cleanly
set_property PACKAGE_PIN Y11  [get_ports oled_pin1_io]; # CS dummy
set_property IOSTANDARD LVCMOS33 [get_ports oled_pin1_io]

set_property PACKAGE_PIN AA11 [get_ports oled_pin3_io]; # MISO dummy
set_property IOSTANDARD LVCMOS33 [get_ports oled_pin3_io]
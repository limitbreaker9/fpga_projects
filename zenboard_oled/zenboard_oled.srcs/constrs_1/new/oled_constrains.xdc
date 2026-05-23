# ZedBoard Onboard OLED Pin Constraints 
set_property PACKAGE_PIN U10 [get_ports {oled[0]}]; # OLED-DC (Data/Command)
set_property PACKAGE_PIN U9  [get_ports {oled[1]}]; # OLED-RES (Reset)
set_property PACKAGE_PIN AB12 [get_ports {oled[2]}]; # OLED-SCLK (Serial Clock)
set_property PACKAGE_PIN AA12 [get_ports {oled[3]}]; # OLED-SDIN (Serial Data)
set_property PACKAGE_PIN U11 [get_ports {oled[4]}]; # OLED-VBAT (Power VBAT)
set_property PACKAGE_PIN U12 [get_ports {oled[5]}]; # OLED-VDD (Power VDD)

# Set IO standards to 3.3V for all 6 pins using the matching port name pattern
set_property IOSTANDARD LVCMOS33 [get_ports -filter {NAME =~ oled*}]
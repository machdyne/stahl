# set the fuses (unset CLKDIV8 so we run at 8mhz instead of default 1mhz)
# use with caution!
sudo avrdude -c usbasp -p m32u2 -B6 -U lfuse:w:0xde:m -U hfuse:w:0xd9:m -U efuse:w:0xf4:m

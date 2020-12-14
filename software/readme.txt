avrdude -c usbasp -p t85 -U lfuse:w:0xe1:m -U hfuse:w:0xdd:m -U efuse:w:0xff:m -U flash:w:TinyTerminal_t85.hex

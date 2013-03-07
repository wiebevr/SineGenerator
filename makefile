%.rel: %.c %.h
	sdcc -c $<

sin_gen: sin_gen.c lcd.rel
	sdcc sin_gen.c lcd.rel
	packihx sin_gen.ihx > sin_gen.hex

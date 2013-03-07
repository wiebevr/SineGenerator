sin_gen.hex: sine_gen.c
	sdcc $(SDCCCFLAGS) $(ASLINKFLAGS) sin_gen.c
	packihx sin_gen.ihx > sin_gen.hex
